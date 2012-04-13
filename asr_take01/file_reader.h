
#ifndef FILE_READER_H_
#define FILE_READER_H_

#include <string>

namespace asr_take01
{
	class FileReader
	{
	protected:
		long long size;
		int channels;
		int samples_per_second;
		int bits_per_sample;
	public:
		virtual void OpenFile(std::string filename) = 0;
		virtual int Read(char* block, unsigned int block_size) = 0;
		virtual void Close() = 0;
		long long Size();
		int Channels();
		int BitsPerSample();
		int SamplesPerSecond();
		void SetChannels(int channels);
		void SetBitsPerSample(int bits_per_sample);
		void SetSamplesPerSecond(int samples_per_second);
	};
}

#endif
