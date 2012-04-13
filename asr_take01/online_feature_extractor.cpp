
#include "online_feature_extractor.h"

#include "audio_capture.h"
#include "mfcc.h"

namespace asr_take01
{
	OnlineFeatureExtractor::OnlineFeatureExtractor(int _channels, int _bits_per_sample, int _samples_per_second) 
		: channels(_channels), bits_per_sample(_bits_per_sample), samples_per_second(_samples_per_second)
	{
		mfcc = new dsp::MFCC();
	}

	OnlineFeatureExtractor::~OnlineFeatureExtractor()
	{
		delete mfcc;
	}

	void OnlineFeatureExtractor::BlockRead(char *block, int block_size)
	{
		running = true;
		std::vector<double *> cep_coeffs;
		//mfcc->Process(block, block_size, cep_coeffs);
		running = false;
	}
}
