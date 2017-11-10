#include <audio/SoundManager.hpp>

#include "audio/alCheck.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

// Rename some functions for older libavcodec/ffmpeg versions (e.g. Ubuntu Trusty)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free   avcodec_free_frame
#endif

#include <array>
#include <iostream>
#include <fstream>

SoundManager::SoundManager() {
    initializeOpenAL();
    initializeAVCodec();
}

SoundManager::~SoundManager() {
    // De-initialize OpenAL
    if (alContext) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(alContext);
    }

    if (alDevice) alcCloseDevice(alDevice);
}

bool SoundManager::initializeOpenAL() {
    alDevice = alcOpenDevice(NULL);
    if (!alDevice) {
        std::cerr << "Could not find OpenAL device!" << std::endl;
        return false;
    }

    alContext = alcCreateContext(alDevice, NULL);
    if (!alContext) {
        std::cerr << "Could not create OpenAL context!" << std::endl;
        return false;
    }

    if (!alcMakeContextCurrent(alContext)) {
        std::cerr << "Unable to make OpenAL context current!" << std::endl;
        return false;
    }

    return true;
}

bool SoundManager::initializeAVCodec() {
    av_register_all();

#if RW_DEBUG && RW_VERBOSE_DEBUG_MESSAGES
        av_log_set_level(AV_LOG_WARNING);
#else
        av_log_set_level(AV_LOG_ERROR);
#endif

    return true;
}

void SoundManager::SoundSource::loadFromFile(const std::string& filename) {
    // Allocate audio frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Error allocating the audio frame" << std::endl;
        return;
    }

    // Allocate formatting context
    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        av_frame_free(&frame);
        std::cerr << "Error opening audio file (" << filename << ")" << std::endl;
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        std::cerr << "Error finding audio stream info" << std::endl;
        return;
    }

    // Find the audio stream
    AVCodec* codec = nullptr;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (streamIndex < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        std::cerr << "Could not find any audio stream in the file " << filename << std::endl;
        return;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];
    AVCodecContext* codecContext = audioStream->codec;
    codecContext->codec = codec;

    // Open the codec
    if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        std::cerr << "Couldn't open the audio codec context" << std::endl;
        return;
    }

    // Expose audio metadata
    channels = codecContext->channels;
    sampleRate = codecContext->sample_rate;

    // OpenAL only supports mono or stereo, so error on more than 2 channels
    if(channels > 2) {
        std::cerr << "Audio has more than two channels" << std::endl;
        av_frame_free(&frame);
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);
        return;
    }

    // Right now we only support signed 16-bit audio
    if(codecContext->sample_fmt != AV_SAMPLE_FMT_S16P) {
        std::cerr << "Audio data isn't in a planar signed 16-bit format" << std::endl;
        av_frame_free(&frame);
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);
        return;
    }

    // Start reading audio packets
    AVPacket readingPacket;
    av_init_packet(&readingPacket);

    while (av_read_frame(formatContext, &readingPacket) == 0) {
        if (readingPacket.stream_index == audioStream->index) {
            AVPacket decodingPacket = readingPacket;

            while (decodingPacket.size > 0) {
                // Decode audio packet
                int gotFrame = 0;
                int len = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

                if (len >= 0 && gotFrame) {
                    // Write samples to audio buffer

                    for(size_t i = 0; i < static_cast<size_t>(frame->nb_samples); i++) {
                        // Interleave left/right channels
                        for(size_t channel = 0; channel < channels; channel++) {
                            int16_t sample = reinterpret_cast<int16_t *>(frame->data[channel])[i];
                            data.push_back(sample);
                        }
                    }

                    decodingPacket.size -= len;
                    decodingPacket.data += len;
                }
                else {
                    decodingPacket.size = 0;
                    decodingPacket.data = nullptr;
                }
            }
        }
        av_free_packet(&readingPacket);
    }

    // Cleanup
    av_frame_free(&frame);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);
}

SoundManager::SoundBuffer::SoundBuffer() {
    alCheck(alGenSources(1, &source));
    alCheck(alGenBuffers(1, &buffer));

    alCheck(alSourcef(source, AL_PITCH, 1));
    alCheck(alSourcef(source, AL_GAIN, 1));
    alCheck(alSource3f(source, AL_POSITION, 0, 0, 0));
    alCheck(alSource3f(source, AL_VELOCITY, 0, 0, 0));
    alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
}

bool SoundManager::SoundBuffer::bufferData(SoundSource& soundSource) {
    alCheck(alBufferData(
        buffer, soundSource.channels == 1 ? AL_FORMAT_MONO16
                                          : AL_FORMAT_STEREO16,
        &soundSource.data.front(), soundSource.data.size() * sizeof(int16_t),
        soundSource.sampleRate));
    alCheck(alSourcei(source, AL_BUFFER, buffer));

    return true;
}

bool SoundManager::loadSound(const std::string& name,
                             const std::string& fileName) {
    Sound* sound = nullptr;
    auto sound_iter = sounds.find(name);

    if (sound_iter != sounds.end()) {
        sound = &sound_iter->second;
    } else {
        auto emplaced = sounds.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(name),
                                       std::forward_as_tuple());
        sound = &emplaced.first->second;

        sound->source.loadFromFile(fileName);
        sound->isLoaded = sound->buffer.bufferData(sound->source);
    }

    return sound->isLoaded;
}
bool SoundManager::isLoaded(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        return sounds[name].isLoaded;
    }

    return false;
}
void SoundManager::playSound(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        alCheck(alSourcePlay(sounds[name].buffer.source));
    }
}
void SoundManager::pauseSound(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        alCheck(alSourcePause(sounds[name].buffer.source));
    }
}
bool SoundManager::isPlaying(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        ALint sourceState;
        alCheck(alGetSourcei(sounds[name].buffer.source, AL_SOURCE_STATE,
                             &sourceState));
        return AL_PLAYING == sourceState;
    }

    return false;
}

bool SoundManager::playBackground(const std::string& fileName) {
    if (this->loadSound(fileName, fileName)) {
        backgroundNoise = fileName;
        this->playSound(fileName);
        return true;
    }

    return false;
}

bool SoundManager::loadMusic(const std::string& name,
                             const std::string& fileName) {
    return loadSound(name, fileName);
}

void SoundManager::playMusic(const std::string& name) {
    playSound(name);
}
void SoundManager::stopMusic(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        alCheck(alSourceStop(sounds[name].buffer.source));
    }
}

void SoundManager::pause(bool p) {
    if (backgroundNoise.length() > 0) {
        if (p) {
            pauseSound(backgroundNoise);
        } else {
            playSound(backgroundNoise);
        }
    }
}
