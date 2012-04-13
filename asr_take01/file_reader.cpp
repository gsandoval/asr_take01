
#include "file_reader.h"

namespace asr_take01
{
	long long FileReader::Size()
	{
		return size;
	}

	int FileReader::BitsPerSample()
	{
		return bits_per_sample;
	}

	int FileReader::SamplesPerSecond()
	{
		return samples_per_second;
	}

	int FileReader::Channels()
	{
		return channels;
	}

	void FileReader::SetChannels(int channels)
	{
		this->channels = channels;
	}

	void FileReader::SetBitsPerSample(int bits_per_sample)
	{
		this->bits_per_sample = bits_per_sample;
	}

	void FileReader::SetSamplesPerSecond(int samples_per_second)
	{
		this->samples_per_second = samples_per_second;
	}
}