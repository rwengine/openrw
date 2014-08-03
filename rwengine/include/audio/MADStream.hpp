#pragma once
#ifndef _MADSTREAM_HPP_
#define _MADSTREAM_HPP_
#include <mad.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <SFML/Audio.hpp>
#include <stdint.h>

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
	sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
	sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

#include <vector>

class MADStream : public sf::SoundStream
{
	mad_decoder mDecoder;
	unsigned int mMadSampleRate;
	unsigned int mMadChannels;
	unsigned char* mFdm;
	struct stat mStat;
	unsigned int mReadProgress;
	std::vector<int16_t> mCurrentSamples;

	static mad_flow ms_header(void* user, mad_header const* header)
	{
		MADStream* stream = static_cast<MADStream*>(user);

		stream->mMadSampleRate = header->samplerate;
		// see enum mad_mode
		stream->mMadChannels = header->mode + 1;

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow ms_input(void* user, mad_stream* stream)
	{
		MADStream* self = static_cast<MADStream*>(user);

		if(! self->mReadProgress ) {
			return MAD_FLOW_STOP;
		}

		auto rd = self->mReadProgress;
		self->mReadProgress = 0;

		mad_stream_buffer(stream, self->mFdm, rd);

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow ms_output(void* user, mad_header const* header,
							  mad_pcm* pcm)
	{
		MADStream* self = static_cast<MADStream*>(user);

		int nsamples = pcm->length;
		mad_fixed_t const *left, *right;

		left = pcm->samples[0];
		right = pcm->samples[1];

		int s = 0;
		while( (nsamples) -- ) {
			signed int sample = *left++;
			self->mCurrentSamples.push_back(scale(sample));

			sample = *right++;
			self->mCurrentSamples.push_back(scale(sample));
			s++;
		}

		return MAD_FLOW_CONTINUE;
	}

	static mad_flow ms_error(void* user, mad_stream* stream, mad_frame* frame)
	{
		sf::err() << "libmad error: " << mad_stream_errorstr(stream);
		return MAD_FLOW_BREAK;
	}

	virtual bool onGetData(sf::SoundStream::Chunk& data)
	{
		data.samples = mCurrentSamples.data();
		data.sampleCount = mCurrentSamples.size();

		return false;
	}

	virtual void onSeek(sf::Time timeOffset)
	{
		/// @todo support seeking.
	}

public:

	MADStream()
		: mFdm(nullptr)
	{

	}

	~MADStream()
	{
		if( mFdm )
		{
			munmap( mFdm, mStat.st_size );
		}
		mad_decoder_finish(&mDecoder);
	}

	bool open(const std::string& loc)
	{
		if( mFdm ) {
			munmap( mFdm, mStat.st_size );
			mCurrentSamples.clear();
			mad_decoder_finish(&mDecoder);
		}

		int fd = ::open(loc.c_str(), O_RDONLY);

		if( fstat(fd, &mStat) == -1 || mStat.st_size == 0) {
			sf::err() << "Fstat failed\n";
		}

		void* m = mmap(0, mStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if( m == MAP_FAILED ) {
			sf::err() << "mmap failed\n";
		}
		mFdm = (unsigned char*)m;
		mReadProgress = mStat.st_size;

		mad_decoder_init(&mDecoder, this,
			ms_input, ms_header, 0, ms_output, ms_error, 0);

		mad_decoder_run(&mDecoder, MAD_DECODER_MODE_SYNC);

		this->initialize(2, mMadSampleRate);

		return true;
	}
};

#endif
