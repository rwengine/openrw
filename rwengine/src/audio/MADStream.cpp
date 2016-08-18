#include "audio/MADStream.hpp"

#include <thread>

inline signed int MADStream::scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE) {
    sample = MAD_F_ONE - 1;
  } else if (sample < -MAD_F_ONE) {
    sample = -MAD_F_ONE;
  }

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

mad_flow MADStream::ms_header(void* user, mad_header const* header)
{
  MADStream* stream = static_cast<MADStream*>(user);

  stream->mMadSampleRate = header->samplerate;
  // see enum mad_mode
  stream->mMadChannels = header->mode + 1;

  return MAD_FLOW_CONTINUE;
}

mad_flow MADStream::ms_input(void* user, mad_stream* stream)
{
  MADStream* self = static_cast<MADStream*>(user);

  if (!self->mReadProgress) {
    return MAD_FLOW_STOP;
  }

  auto rd = self->mReadProgress;
  self->mReadProgress = 0;

  mad_stream_buffer(stream, self->mFdm, rd);

  return MAD_FLOW_CONTINUE;
}

mad_flow MADStream::ms_output(void* user, mad_header const* header, mad_pcm* pcm)
{
  RW_UNUSED(header);

  MADStream* self = static_cast<MADStream*>(user);

  if (self->stopped) {
    return MAD_FLOW_STOP;
  }

  if (!self->numFreeBuffers) {
    ALint buffersProcessed;
    do {
      /**
       * Sleep a bit while waiting for OpenAL buffers to become available.
       * The number is arbitrary and depends on the size of the buffer/audio samples,
       * as well as how quickly the computer can feed more buffers into OpenAL.
       */
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      alGetSourcei(self->alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
    } while (buffersProcessed <= 0);

    alCheck(alSourceUnqueueBuffers(self->alSource, buffersProcessed, self->unqueuedBuffers));
    self->numFreeBuffers += buffersProcessed;
  }

  int nsamples = pcm->length;
  mad_fixed_t const* left, *right;

  left = pcm->samples[0];
  right = pcm->samples[1];

  int s = 0;
  while (nsamples--) {
    signed int sample = *left++;
    self->mCurrentSamples.push_back(scale(sample));

    sample = *right++;
    self->mCurrentSamples.push_back(scale(sample));
    s++;
  }

  alCheck(alBufferData(self->buffers[self->currentBuffer], AL_FORMAT_STEREO16,
                       self->mCurrentSamples.data(),
                       self->mCurrentSamples.size() * sizeof(uint16_t), pcm->samplerate));
  alCheck(alSourceQueueBuffers(self->alSource, 1, self->buffers + self->currentBuffer));

  self->mCurrentSamples.clear();
  self->currentBuffer++;
  self->currentBuffer %= numALbuffers;
  self->numFreeBuffers--;

  return MAD_FLOW_CONTINUE;
}

mad_flow MADStream::ms_error(void* user, mad_stream* stream, mad_frame* frame)
{
  RW_UNUSED(user);
  RW_UNUSED(frame);

  std::cerr << "libmad error: " << mad_stream_errorstr(stream) << std::endl;
  return MAD_FLOW_BREAK;
}

MADStream::MADStream() : mFdm(nullptr)
{
  alCheck(alGenBuffers(numALbuffers, buffers));
  alCheck(alGenSources(1, &alSource));
}

MADStream::~MADStream()
{
  if (mFdm) {
    munmap(mFdm, mStat.st_size);
    mad_decoder_finish(&mDecoder);
  }
}

bool MADStream::openFromFile(const std::string& loc)
{
  if (mFdm) {
    munmap(mFdm, mStat.st_size);
    mCurrentSamples.clear();
    mad_decoder_finish(&mDecoder);
  }

  int fd = ::open(loc.c_str(), O_RDONLY);

  if (fstat(fd, &mStat) == -1 || mStat.st_size == 0) {
    std::cerr << "Fstat failed (" << loc << ")" << std::endl;
    return false;
  }

  void* m = mmap(0, mStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (m == MAP_FAILED) {
    std::cerr << "mmap failed (" << loc << ")" << std::endl;
    return false;
  }

  mFdm = (unsigned char*)m;
  mReadProgress = mStat.st_size;

  mad_decoder_init(&mDecoder, this, ms_input, ms_header, 0, ms_output, ms_error, 0);

  new std::thread([&]() { mad_decoder_run(&mDecoder, MAD_DECODER_MODE_SYNC); });

  alCheck(alSourcef(alSource, AL_PITCH, 1));
  alCheck(alSourcef(alSource, AL_GAIN, 1));
  alCheck(alSource3f(alSource, AL_POSITION, 0, 0, 0));
  alCheck(alSource3f(alSource, AL_VELOCITY, 0, 0, 0));
  alCheck(alSourcei(alSource, AL_LOOPING, AL_FALSE));

  return true;
}

void MADStream::play() { alCheck(alSourcePlay(alSource)); }

void MADStream::stop()
{
  stopped = true;
  alCheck(alSourcePlay(alSource));
}
