
#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include "raw_audio_listener.h"

namespace dsp
{
	class MFCC;
}

namespace asr_take01
{
	class OnlineFeatureExtractor : public audiocapture::RawAudioListener
	{
	private:
		dsp::MFCC *mfcc;
		bool running;
		int channels;
		int bits_per_sample;
		int samples_per_second;
	public:
		OnlineFeatureExtractor(int channels, int bits_per_sample, int samples_per_second);
		virtual ~OnlineFeatureExtractor();
		void BlockRead(char *block, int block_size);
	};
}

#endif
