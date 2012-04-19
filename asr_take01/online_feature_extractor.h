
#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include "raw_audio_listener.h"

#include <vector>
#include <Windows.h>

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
		bool running, stopped;
		int channels;
		int bits_per_sample;
		int samples_per_second;
		std::vector<FeatureListener *> listeners;
		int feature_count;
		int buffer_size;
		void *thread_handle;
		void *queue_wait;
		unsigned long thread_id;
		char** queue;
		int* queue_size;
		int queue_top, queue_bottom;
		CRITICAL_SECTION queue_mutex;
		int QueueSize();
		int vectors_per_sample;
	public:
		OnlineFeatureExtractor(int feature_count, int channels, int bits_per_sample, int samples_per_second, int buffer_size,
			int fft_length, int mel_filter_bank_size, int frame_length, int frame_shift, int vectors_per_sample);
		virtual ~OnlineFeatureExtractor();
		void BlockRead(char *block, int block_size);
		void AddFeatureListener(FeatureListener *listener);
		void Run();
	};
}

#endif
