
#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include "raw_audio_listener.h"

#include <vector>

namespace dsp
{
	class MFCC;
}

namespace asr_take01
{
	class FeatureListener;

	class OnlineFeatureExtractor : public audiocapture::RawAudioListener
	{
	private:
		dsp::MFCC *mfcc;
		bool running;
		int channels;
		int bits_per_sample;
		int samples_per_second;
		std::vector<FeatureListener *> listeners;
		int feature_count;
	public:
		OnlineFeatureExtractor(int feature_count, int channels, int bits_per_sample, int samples_per_second);
		virtual ~OnlineFeatureExtractor();
		void BlockRead(char *block, int block_size);
		void AddFeatureListener(FeatureListener *listener);
	};
}

#endif
