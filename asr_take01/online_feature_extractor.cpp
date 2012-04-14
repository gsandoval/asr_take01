
#include "online_feature_extractor.h"

#include "audio_capture.h"
#include "mfcc.h"
#include "feature_listener.h"

namespace asr_take01
{
	using namespace std;

	OnlineFeatureExtractor::OnlineFeatureExtractor(int _feature_count, int _channels, int _bits_per_sample, int _samples_per_second) 
		: channels(_channels), bits_per_sample(_bits_per_sample), samples_per_second(_samples_per_second), feature_count(_feature_count)
	{
		mfcc = new dsp::MFCC();
		mfcc->SetCepstralCoefficientsNumber(_feature_count);
		mfcc->SetNoCoefficientZero(true);
		mfcc->SetSamplingFrequency(samples_per_second);
		mfcc->SetFrameLength(samples_per_second / 10 * 1);
		mfcc->SetFrameShift(samples_per_second / 20 * 1);
		mfcc->SetFFTFrameLength(1024);
		mfcc->Init();
	}

	void OnlineFeatureExtractor::AddFeatureListener(FeatureListener *listener)
	{
		listeners.push_back(listener);
	}

	OnlineFeatureExtractor::~OnlineFeatureExtractor()
	{
		delete mfcc;
	}

	void OnlineFeatureExtractor::BlockRead(char *block, int block_size)
	{
		running = true;

		int offset = 200;
		int coeff_needed = 11;
		for (int d = 0; d < block_size / offset; ++d) {
			vector<float *> cep_coeffs;
			if (block_size - (d * offset) < 9600) {
				break;
			}
			mfcc->Process(block + (d * offset), block_size - (d * offset), cep_coeffs);
			if (coeff_needed <= cep_coeffs.size()) {
				for (unsigned int i = 0; i < cep_coeffs.size() - coeff_needed; ++i) {
					vector<double> feature;
					for (int j = 0; j < coeff_needed; ++j) {
						for (int k = 0; k < feature_count; ++k) {
							feature.push_back(cep_coeffs[i + j][k]);
						}
					}
					for (unsigned int j = 0; j < listeners.size(); ++j) {
						listeners[j]->FeatureReady(feature);
					}
				}
			}
			for (unsigned int i = 0; i < cep_coeffs.size(); ++i) {
				delete[] cep_coeffs[i];
			}
		}

		running = false;
	}
}
