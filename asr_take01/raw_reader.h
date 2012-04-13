
#ifndef RAW_READER_H_
#define RAW_READER_H_

#include "file_reader.h"

#include <string>
#include <fstream>

namespace asr_take01
{
	class RawReader : public FileReader
	{
	private:
		std::ifstream file;
	public:
		void OpenFile(std::string filename);
		int Read(char* block, unsigned int block_size);
		void Close();
		unsigned int Size();
		int Channels();
		int BitsPerSample();
		int SamplesPerSecond();
	};
}

#endif
