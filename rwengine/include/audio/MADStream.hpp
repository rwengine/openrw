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
#include <iostream>
#include <rw/defines.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include "audio/alCheck.hpp"


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

	constexpr static size_t numALbuffers = 8;
	ALuint buffers[numALbuffers];
	size_t currentBuffer = 0;
	ALuint alSource;

	static inline signed int scale(mad_fixed_t sample);
	static mad_flow ms_header(void* user, mad_header const* header);
	static mad_flow ms_input(void* user, mad_stream* stream);
	static mad_flow ms_output(void* user, mad_header const* header, mad_pcm* pcm);
	static mad_flow ms_error(void* user, mad_stream* stream, mad_frame* frame);

	virtual bool onGetData(sf::SoundStream::Chunk& data);

	virtual void onSeek(sf::Time timeOffset);

public:

	MADStream();
	~MADStream();

	bool openFromFile(const std::string& loc);
};

#endif
