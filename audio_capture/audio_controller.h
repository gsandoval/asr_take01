
#ifndef AUDIO_CONTROLLER_H_
#define AUDIO_CONTROLLER_H_

#include <vector>

#include <Windows.h>

namespace audiocapture
{
	class RawAudioListener;
	class AudioDevice;

	void CALLBACK wave_in_proc(HWAVEIN hwi, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);

	class AudioController
	{
	public:
		AudioController(int channels, int samples_per_second, int bits_per_sample, int buffer_count, int buffer_size);
		virtual ~AudioController();
		std::vector<AudioDevice*> ListDevices();
		int BufferSize();
		bool SelectDevice(AudioDevice*);
		bool Start();
		bool Stop();
		void Reset();
		void AddRawAudioListener(RawAudioListener*);
		friend void CALLBACK wave_in_proc(HWAVEIN hwi, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);
	private:
		std::vector<AudioDevice*> devices;
		std::vector<WAVEHDR *> buffers;
		bool capturing;
		int channels;
		int samples_per_second;
		int bits_per_sample;
		int buffer_size;
		int buffer_count;
		HWAVEIN wave_in_handle;
		//HWAVEOUT wave_out_handle;
		void AddBuffer(WAVEHDR *buffer);
		void ReleaseBuffer(WAVEHDR *buffer);
		std::vector<RawAudioListener *> raw_audio_listeners;
		void NotifyListeners(WAVEHDR *buffer);
	};
}

#endif
