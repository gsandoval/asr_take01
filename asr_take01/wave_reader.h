
#ifndef WAVE_READER_H_
#define WAVE_READER_H_

#include "file_reader.h"

#include <string>
#include <Windows.h>

namespace asr_take01
{
	class WaveReader : public FileReader
	{
	private:
		HMMIO file_handle;
	public:
		void OpenFile(std::string filename);
		int Read(char* block, unsigned int block_size);
		void Close();
	};
}

#endif
