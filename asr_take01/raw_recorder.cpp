#include "raw_recorder.h"

namespace asr_take01
{
	using namespace std;

	RawRecorder::RawRecorder()
	{
	}

	RawRecorder::~RawRecorder()
	{
	}

	void RawRecorder::CreateOutputFile(std::string filename)
	{
		file.open(filename, ios::out | ios::binary);
	}

	void RawRecorder::CloseOutputFile()
	{
		file.close();
	}

	void RawRecorder::BlockRead(char *block, int block_size)
	{
		file.write(block, block_size);
	}
}
