
#include "online_feature_extractor.h"

#include "audio_capture.h"
#include "mfcc.h"
#include "feature_listener.h"

#include <iostream>

namespace asr_take01
{
	using namespace std;

	const int QUEUE_SIZE = 15;

	unsigned long __stdcall thread_entry_point(void* param)
	{
		OnlineFeatureExtractor *ofe = reinterpret_cast<OnlineFeatureExtractor *>(param);
		ofe->Run();
		return 0;
	}

	int OnlineFeatureExtractor::QueueSize()
	{
		return queue_top >= queue_bottom? queue_top - queue_bottom : QUEUE_SIZE - queue_bottom + queue_top;
	}

	void OnlineFeatureExtractor::Run()
	{
		running = true;

		int offset = 200;
		int frame_size = 9600;
		unsigned int coeff_needed = 11;
		char *working_buffer = new char[buffer_size * (QUEUE_SIZE + 2)];
		int bottom = 0;
		int top = 0;
		while (running) {
			WaitForSingleObject(queue_wait, INFINITE);
			EnterCriticalSection(&queue_mutex);
			if (queue_top == queue_bottom) {
				LeaveCriticalSection(&queue_mutex);
				continue;
			}
			for (int i = 0; i < QueueSize(); ++i) {
				memcpy(working_buffer + top, queue[queue_bottom], queue_size[queue_bottom]);
				top += queue_size[queue_bottom];
				queue_bottom = ++queue_bottom % QUEUE_SIZE;
			}
			LeaveCriticalSection(&queue_mutex);

			vector<float *> cep_coeffs;
			mfcc->Process(working_buffer, top, cep_coeffs);
			bool found_something = false;
			if (coeff_needed <= cep_coeffs.size()) {
				for (unsigned int i = 0; i < cep_coeffs.size() - coeff_needed && !found_something; ++i) {
					vector<double> feature;
					for (unsigned int j = 0; j < coeff_needed; ++j) {
						for (int k = 0; k < feature_count; ++k) {
							feature.push_back(cep_coeffs[i + j][k]);
						}
					}
					for (unsigned int j = 0; j < listeners.size(); ++j) {
						//listeners[j]->FeatureReady(feature);
						found_something = listeners[j]->FeatureReady(feature);
					}
				}
			}
			for (unsigned int i = 0; i < cep_coeffs.size(); ++i) {
				delete[] cep_coeffs[i];
			}
			if (found_something) {
				top = bottom = 0;
			} else {
				int keep = (frame_size - offset);
				for (int i = 0; i < keep; ++i) {
					working_buffer[i] = working_buffer[top - keep + i];
				}
				bottom = 0;
				top = keep;
			}
		}
		delete[] working_buffer;

		stopped = true;
	}

	OnlineFeatureExtractor::OnlineFeatureExtractor(int _feature_count, int _channels, int _bits_per_sample, int _samples_per_second, 
		int _buffer_size, int fft_length, int mel_filter_bank_size) : channels(_channels), bits_per_sample(_bits_per_sample), 
		samples_per_second(_samples_per_second), feature_count(_feature_count), running(false), stopped(false), buffer_size(_buffer_size)
	{
		mfcc = new dsp::MFCC();
		mfcc->SetCepstralCoefficientsNumber(_feature_count);
		mfcc->SetNoCoefficientZero(true);
		mfcc->SetSamplingFrequency(samples_per_second);
		mfcc->SetFrameLength(samples_per_second / 10 * 1);
		mfcc->SetFrameShift(samples_per_second / 20 * 1);
		mfcc->SetFFTFrameLength(fft_length);
		mfcc->SetMelFilterBankSize(mel_filter_bank_size);
		mfcc->Init();

		queue_top = queue_bottom = 0;
		queue_size = new int[QUEUE_SIZE];
		queue = new char*[QUEUE_SIZE];
		for (int i = 0; i < QUEUE_SIZE; ++i) {
			queue[i] = new char[_buffer_size];
		}

		queue_wait = CreateEvent(0, false, false, 0);

		InitializeCriticalSection(&queue_mutex);
		thread_handle = CreateThread(0, 0, thread_entry_point, this, 0, &thread_id);
	}

	void OnlineFeatureExtractor::AddFeatureListener(FeatureListener *listener)
	{
		listeners.push_back(listener);
	}

	OnlineFeatureExtractor::~OnlineFeatureExtractor()
	{
		running = false;
		EnterCriticalSection(&queue_mutex);
		SetEvent(queue_wait);
		LeaveCriticalSection(&queue_mutex);

		delete mfcc;

		while (!stopped) {
			Sleep(200);
		}

		delete[] queue_size;
		for (int i = 0; i < QUEUE_SIZE; ++i) {
			delete[] queue[i];
		}
		delete[] queue;

		CloseHandle(thread_handle);
		CloseHandle(queue_wait);
		DeleteCriticalSection(&queue_mutex);
	}

	void OnlineFeatureExtractor::BlockRead(char *block, int block_size)
	{
		EnterCriticalSection(&queue_mutex);
		memcpy(queue[queue_top], block, block_size);
		queue_size[queue_top] = block_size;
		queue_top = ++queue_top % QUEUE_SIZE;
		SetEvent(queue_wait);
		LeaveCriticalSection(&queue_mutex);
	}
}
