
#include "wave_recorder.h"

namespace asr_take01
{
	WaveRecorder::WaveRecorder() : opened_file(false)
	{
	}

	WaveRecorder::~WaveRecorder()
	{
	}

	void WaveRecorder::CreateOutputFile(std::string filename)
	{
		wchar_t *file = new wchar_t[filename.size() + 1];
		memset(file, 0, filename.size() + 1);
		MultiByteToWideChar(CP_ACP, NULL, filename.c_str(), -1, file, filename.size() + 1);
		file_handle = mmioOpen(file, 0, MMIO_CREATE | MMIO_WRITE);
		delete[] file;

		chunk_info.fccType = mmioStringToFOURCC(L"WAVE", 0);
		MMRESULT mr = mmioCreateChunk(file_handle, &chunk_info, MMIO_CREATERIFF);

		MMCKINFO wave_fmt;
		wave_fmt.ckid = mmioStringToFOURCC(L"fmt ", 0);
		wave_fmt.cksize = sizeof(WAVEFORMATEX);
		mr = mmioCreateChunk(file_handle, &wave_fmt, 0);
		WAVEFORMATEX header;
		header.wFormatTag = WAVE_FORMAT_PCM;
		header.nChannels = channels;
		header.wBitsPerSample = bits_per_sample;
		header.nSamplesPerSec = samples_per_second;
		header.nBlockAlign = header.nChannels * (header.wBitsPerSample / 8);
		header.nAvgBytesPerSec = header.nSamplesPerSec * header.nBlockAlign;
		header.cbSize = 0;
		long bytes = mmioWrite(file_handle, (char*) &header, sizeof(WAVEFORMATEX) - 2);
		mr = mmioAscend(file_handle, &wave_fmt, 0);

		memset(&data_chunk_info, 0, sizeof(MMCKINFO));
		data_chunk_info.ckid = mmioStringToFOURCC(L"data", 0);
		//data_chunk_info.cksize = block_size;
		mr = mmioCreateChunk(file_handle, &data_chunk_info, 0);

		opened_file = true;
	}

	void WaveRecorder::CloseOutputFile()
	{
		opened_file = false;
		MMRESULT mr = mmioAscend(file_handle, &data_chunk_info, 0);
		mr = mmioAscend(file_handle, &chunk_info, 0);
		mr = mmioClose(file_handle, 0);
	}

	void WaveRecorder::BlockRead(char *block, int block_size)
	{
		if (opened_file) {
			long bytes = mmioWrite(file_handle, block, block_size);
		}
	}
}