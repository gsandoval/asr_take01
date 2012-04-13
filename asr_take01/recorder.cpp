#include "recorder.h"

namespace asr_take01
{
	Recorder::~Recorder()
	{
	}

	void Recorder::SetChannels(int channels)
	{
		this->channels = channels;
	}

	void Recorder::SetBitsPerSample(int bits_per_sample)
	{
		this->bits_per_sample = bits_per_sample;
	}

	void Recorder::SetSamplesPerSecond(int samples_per_second)
	{
		this->samples_per_second = samples_per_second;
	}
}
