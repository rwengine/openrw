#include "audio/alCheck.hpp"
#include "audio/Sound.hpp"
#include <iostream>

// Rename some functions for older libavcodec/ffmpeg versions (e.g. Ubuntu Trusty)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free   avcodec_free_frame
#endif

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(57,80,100)
#define avio_context_free av_freep
#endif

#define OUTPUT_CHANNELS 2
#define BUFFER_SIZE 192000
#define OUTPUT_BITS 16
#define OUTPUT_FMT AV_SAMPLE_FMT_S16

SoundBuffer::SoundBuffer() {
    alCheck(alGenSources(1, &source));
    alCheck(alGenBuffers(1, &buffer));

    alCheck(alSourcef(source, AL_PITCH, 1));
    alCheck(alSourcef(source, AL_GAIN, 1));
    alCheck(alSource3f(source, AL_POSITION, 0, 0, 0));
    alCheck(alSource3f(source, AL_VELOCITY, 0, 0, 0));
    alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
}

bool SoundBuffer::bufferData(SoundSource& soundSource) {
    alCheck(alBufferData(
                buffer, soundSource.channels == 1 ? AL_FORMAT_MONO16
                                                  : AL_FORMAT_STEREO16,
                &soundSource.data.front(), soundSource.data.size() * sizeof(int16_t),
                soundSource.sampleRate));
    alCheck(alSourcei(source, AL_BUFFER, buffer));

    return true;
}

void SoundSource::loadFromFile(const std::string& filename) {
    // Allocate audio frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        RW_ERROR("Error allocating the audio frame");
        return;
    }

    // Allocate formatting context
    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr) != 0) {
        av_frame_free(&frame);
        RW_ERROR("Error opening audio file (" << filename << ")");
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Error finding audio stream info");
        return;
    }

    // Find the audio stream
    //AVCodec* codec = nullptr;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Could not find any audio stream in the file ");
        return;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];
    AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,5,0)
    AVCodecContext* codecContext = audioStream->codec;
    codecContext->codec = codec;

    // Open the codec
    if (avcodec_open2(codecContext, codecContext->codec, nullptr) != 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't open the audio codec context");
        return;
    }
#else
    // Initialize codec context for the decoder.
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't allocate a decoding context.");
        return;
    }

    // Fill the codecCtx with the parameters of the codec used in the read file.
    if (avcodec_parameters_to_context(codecContext, audioStream->codecpar) != 0) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't find parametrs for context");
    }

    // Initialize the decoder.
    if (avcodec_open2(codecContext, codec, nullptr) != 0) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't open the audio codec context");
        return;
    }
#endif

    // Expose audio metadata
    channels = OUTPUT_CHANNELS;
    sampleRate = codecContext->sample_rate;

    // OpenAL only supports mono or stereo, so error on more than 2 channels
    if(channels > 2) {
        RW_ERROR("Audio has more than two channels");
        av_frame_free(&frame);
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);
        return;
    }

    // prepare resampler
    SwrContext* swr = nullptr;

    // Start reading audio packets
    AVPacket readingPacket;
    av_init_packet(&readingPacket);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,37,100)

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
#else

    AVFrame* resampled = nullptr;

    while (av_read_frame(formatContext, &readingPacket) == 0) {
        if (readingPacket.stream_index == audioStream->index) {

            int sendPacket = avcodec_send_packet(codecContext, &readingPacket);
            av_packet_unref(&readingPacket);
            int receiveFrame = 0;


            while ((receiveFrame = avcodec_receive_frame(codecContext, frame)) == 0) {
                if(!swr) {
                    if(frame->channels == 1 || frame->channel_layout == 0) {
                        frame->channel_layout = av_get_default_channel_layout(1);
                    }
                    swr = swr_alloc_set_opts(nullptr,
                                             AV_CH_LAYOUT_STEREO,    // output
                                             OUTPUT_FMT,                                        // output
                                             frame->sample_rate,                                       // output
                                             frame->channel_layout,   // input
                                             static_cast<AVSampleFormat>(frame->format),                                      // input
                                             frame->sample_rate,                                            // input
                                             0,
                                             nullptr);
                    if (!swr) {
                        fprintf(stderr, "Resampler has not been properly initialized\n");
                        return;
                    }
                    swr_init(swr);
                    if (!swr_is_initialized(swr)) {
                        fprintf(stderr, "Resampler has not been properly initialized\n");
                        return;
                    }
                }
                // Decode audio packet

                if (receiveFrame == 0 && sendPacket == 0) {
                    // Write samples to audio buffer
                    resampled = av_frame_alloc();

                    resampled->channel_layout = AV_CH_LAYOUT_STEREO;
                    resampled->sample_rate = frame->sample_rate;
                    resampled->format = OUTPUT_FMT;
                    resampled->channels = OUTPUT_CHANNELS;

                    swr_config_frame(swr, resampled, frame);
                    if (swr_convert_frame(swr, resampled, frame) < 0) {
                        std::cout << "Error resampling "<< filename << '\n';
                    }
                    for(size_t i = 0; i < static_cast<size_t>(resampled->nb_samples) * channels; i++) {
                        data.push_back(reinterpret_cast<int16_t *>(resampled->data[0])[i]);
                    }
                }
            }
        }
    }

    av_frame_unref(resampled);
#endif

    // Cleanup
    /// Free all data used by the frame.
    av_frame_free(&frame);

    /// Free resampler
    swr_free(&swr);

    /// Close the context and free all data associated to it, but not the context itself.
    avcodec_close(codecContext);

    /// Free the context itself.
    avcodec_free_context(&codecContext);

    /// We are done here. Close the input.
    avformat_close_input(&formatContext);
}

void SoundSource::loadSfx(const rwfs::path& path, const size_t& index) {
    // Allocate audio frame
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        RW_ERROR("Error allocating the audio frame");
        return;
    }

    // Allocate formatting context
    AVFormatContext* formatContext = nullptr;
    LoaderSDT loader{};
    loader.load(path / "audio/sfx");
    formatContext = loader.loadSound(index);

    if (avformat_open_input(&formatContext, "nothint", nullptr, nullptr) != 0) {
        av_frame_free(&frame);
        RW_ERROR("Error opening audio file (" << index << ")");
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Error finding audio stream info");
        return;
    }

    // Find the audio stream
    //AVCodec* codec = nullptr;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Could not find any audio stream in the file ");
        return;
    }

    AVStream* audioStream = formatContext->streams[streamIndex];
    AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,5,0)
    AVCodecContext* codecContext = audioStream->codec;
    codecContext->codec = codec;

    // Open the codec
    if (avcodec_open2(codecContext, codecContext->codec, nullptr) != 0) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't open the audio codec context");
        return;
    }
#else
    // Initialize codec context for the decoder.
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        av_frame_free(&frame);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't allocate a decoding context.");
        return;
    }

    // Fill the codecCtx with the parameters of the codec used in the read file.
    if (avcodec_parameters_to_context(codecContext, audioStream->codecpar) != 0) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't find parametrs for context");
    }

    // Initialize the decoder.
    if (avcodec_open2(codecContext, codec, nullptr) != 0) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        RW_ERROR("Couldn't open the audio codec context");
        return;
    }
#endif

    // Expose audio metadata
    channels = codecContext->channels;
    sampleRate =loader.assetInfo.sampleRate;

    // OpenAL only supports mono or stereo, so error on more than 2 channels
    if(channels > 2) {
        RW_ERROR("Audio has more than two channels");
        av_frame_free(&frame);
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);
        return;
    }

    // Start reading audio packets
    AVPacket readingPacket;
    av_init_packet(&readingPacket);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,37,100)

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
#else

    while (av_read_frame(formatContext, &readingPacket) == 0) {
        if (readingPacket.stream_index == audioStream->index) {
            AVPacket decodingPacket = readingPacket;

            int sendPacket = avcodec_send_packet(codecContext, &decodingPacket);
            int receiveFrame = 0;

            while ((receiveFrame = avcodec_receive_frame(codecContext, frame)) == 0) {
                // Decode audio packet

                if (receiveFrame == 0 && sendPacket == 0) {
                    // Write samples to audio buffer

                    for(size_t i = 0; i < static_cast<size_t>(frame->nb_samples); i++) {
                        // Interleave left/right channels
                        for(size_t channel = 0; channel < channels; channel++) {
                            int16_t sample = reinterpret_cast<int16_t *>(frame->data[channel])[i];
                            data.push_back(sample);
                        }
                    }
                }
            }
        }
        av_packet_unref(&readingPacket);
    }

#endif

    // Cleanup
    /// Free all data used by the frame.
    av_frame_free(&frame);

    /// Close the context and free all data associated to it, but not the context itself.
    avcodec_close(codecContext);

    /// Free the context itself.
    avcodec_free_context(&codecContext);

    /// Free our custom AVIO.
    av_free(formatContext->pb->buffer);
    avio_context_free(&formatContext->pb);

    /// We are done here. Close the input.
    avformat_close_input(&formatContext);
}

bool SoundBuffer::isPlaying() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE,
                         &sourceState));
    return AL_PLAYING == sourceState;
}

bool SoundBuffer::isPaused() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE,
                         &sourceState));
    return AL_PAUSED == sourceState;
}

bool SoundBuffer::isStoped() const {
    ALint sourceState;
    alCheck(alGetSourcei(source, AL_SOURCE_STATE,
                         &sourceState));
    return AL_STOPPED  == sourceState;
}

void SoundBuffer::play() {
    alCheck(alSourcePlay(source));
}
void SoundBuffer::pause() {
    alCheck(alSourcePause(source));
}
void SoundBuffer::stop() {
    alCheck(alSourceStop(source));
}

void SoundBuffer::setPosition(const glm::vec3 position) {
    alCheck(alSource3f(source, AL_POSITION, position.x, position.y, position.z));
}

void SoundBuffer::setLooping(const bool  looping) {
    if(looping) {
        alCheck(alSourcei(source, AL_LOOPING, AL_TRUE));
    }
    else {
        alCheck(alSourcei(source, AL_LOOPING, AL_FALSE));
    }
}

void SoundBuffer::setPitch(const float  pitch) {
    alCheck(alSourcef(source, AL_PITCH, pitch));
}
void SoundBuffer::setGain(const float  gain) {
    alCheck(alSourcef(source, AL_GAIN, gain));
}
void SoundBuffer::setMaxDistance(const float  maxDist) {
    alCheck(alSourcef(source, AL_MAX_DISTANCE, maxDist));
}


