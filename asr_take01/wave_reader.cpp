
#include "wave_reader.h"

#include <cstdlib>

namespace asr_take01
{
	using namespace std;

	void WaveReader::OpenFile(string filename)
	{
		wchar_t *file = new wchar_t[filename.size() + 1];
		memset(file, 0, filename.size() + 1);
		MultiByteToWideChar(CP_ACP, NULL, filename.c_str(), -1, file, filename.size() + 1);
		file_handle = mmioOpen(file, 0, MMIO_READ);

		MMCKINFO chunk_info;
		memset(&chunk_info, 0, sizeof(MMCKINFO));
		MMRESULT mr = mmioDescend(file_handle, &chunk_info, 0, MMIO_FINDRIFF);

		MMCKINFO fmt_chunk_info;
		fmt_chunk_info.ckid = mmioStringToFOURCC(L"fmt", 0);
		mr = mmioDescend(file_handle, &fmt_chunk_info, &chunk_info, MMIO_FINDCHUNK);
		WAVEFORMATEX wave_format;
		mr = mmioRead(file_handle, (char*) &wave_format, fmt_chunk_info.cksize);

		channels = wave_format.nChannels;
		samples_per_second = wave_format.nSamplesPerSec;
		bits_per_sample = wave_format.wBitsPerSample;

		mr = mmioAscend(file_handle, &fmt_chunk_info, 0);
		MMCKINFO data_chunk_info;
		data_chunk_info.ckid = mmioStringToFOURCC(L"data", 0);
		mr = mmioDescend(file_handle, &data_chunk_info, &chunk_info, MMIO_FINDCHUNK);

		size = data_chunk_info.cksize;
	}

	int WaveReader::Read(char* block, unsigned int block_size)
	{
		return mmioRead(file_handle, block, block_size);
	}

	void WaveReader::Close()
	{
		mmioClose(file_handle, 0);
	}
}
