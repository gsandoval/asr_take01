#ifndef RECORDER_H_
#define RECORDER_H_

#include "raw_audio_listener.h"

namespace asr_take01
{
	class Recorder : public audiocapture::RawAudioListener
	{
	protected:
		int channels;
		int bits_per_sample;
		int samples_per_second;
	public:
		void SetChannels(int channels);
		void SetBitsPerSample(int bits_per_sample);
		void SetSamplesPerSecond(int samples_per_second);
		virtual ~Recorder();
		virtual void CreateOutputFile(std::string filename) = 0;
		virtual void CloseOutputFile() = 0;
		virtual void BlockRead(char *block, int block_size) = 0;
	};
}

#endif
