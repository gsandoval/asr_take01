
#ifndef RAW_RECORDER_H_
#define RAW_RECORDER_H_

#include "recorder.h"

#include <fstream>

namespace asr_take01
{
	class RawRecorder : public Recorder
	{
	private:
		std::ofstream file;
	public:
		RawRecorder();
		virtual ~RawRecorder();
		void CreateOutputFile(std::string filename);
		void CloseOutputFile();
		void BlockRead(char *block, int block_size);
	};
}

#endif
