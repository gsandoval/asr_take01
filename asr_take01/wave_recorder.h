
#ifndef WAVE_RECORDER_H_
#define WAVE_RECORDER_H_

#include "recorder.h"
#include <Windows.h>

#include <string>

namespace asr_take01
{
	class WaveRecorder : public Recorder
	{
	private:
		bool opened_file;
		HMMIO file_handle;
		MMCKINFO chunk_info;
		MMCKINFO data_chunk_info;
	public:
		WaveRecorder();
		virtual ~WaveRecorder();
		void CreateOutputFile(std::string filename);
		void CloseOutputFile();
		void BlockRead(char *block, int block_size);
	};
}

#endif 
