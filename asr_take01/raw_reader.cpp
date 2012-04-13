
#include "raw_reader.h"
#include <fstream>

namespace asr_take01
{
	using namespace std;

	void RawReader::OpenFile(string filename)
	{
		file.open(filename, ios::in | ios::binary | ios::ate);
		size = file.tellg();
		file.seekg(0);
	}

	int RawReader::Read(char* block, unsigned int block_size)
	{
		int before = file.tellg();
		file.read(block, block_size);
		int after = file.tellg();
		return after - before;
	}

	void RawReader::Close()
	{
		file.close();
	}
}
