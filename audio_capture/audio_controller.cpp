#include "audio_controller.h"
#include "audio_device.h"
#include "raw_audio_listener.h"

#include <Windows.h>

namespace audiocapture
{
	void CALLBACK wave_in_proc(HWAVEIN hwi, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2)
	{
		AudioController *ac = reinterpret_cast<AudioController *>(instance);
		WAVEHDR *buffer;
		switch (msg) {
		case MM_WIM_CLOSE:
			break;
		case MM_WIM_DATA:
			buffer = (WAVEHDR *) param1;
			ac->NotifyListeners(buffer);
			if (ac->capturing) {
				ac->AddBuffer(buffer);
			}
			break;
		case MM_WIM_OPEN:
			break;
		}
	}

	AudioController::AudioController(int _channels, int _samples_per_second, int _bits_per_sample, int _buffer_count, int _buffer_size) 
		: capturing(false), channels(_channels), samples_per_second(_samples_per_second), bits_per_sample(_bits_per_sample),
		buffer_size(_buffer_size), buffer_count(_buffer_count)
	{
	}

	AudioController::~AudioController()
	{
		for (unsigned int i = 0; i < devices.size(); ++i) {
			delete devices[i];
		}
	}

	int AudioController::BufferSize()
	{
		return buffer_size;
	}

	void AudioController::AddRawAudioListener(RawAudioListener *listener)
	{
		raw_audio_listeners.push_back(listener);
	}

	void AudioController::NotifyListeners(WAVEHDR *buffer)
	{
		for (unsigned int i = 0; i < raw_audio_listeners.size(); ++i) {
			raw_audio_listeners[i]->BlockRead(buffer->lpData, buffer->dwBytesRecorded);
		}
	}

	void AudioController::AddBuffer(WAVEHDR *buffer)
	{
		waveInAddBuffer(wave_in_handle, buffer, sizeof(WAVEHDR));
	}

	std::vector<AudioDevice*> AudioController::ListDevices()
	{
		int device_count = waveInGetNumDevs();
		WAVEINCAPS wic;
		char friendly_name[130];
		for (int i = 0; i < device_count; ++i) {
			waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS));
			WideCharToMultiByte(CP_ACP, NULL, wic.szPname, -1, friendly_name, 120, NULL, NULL);
			AudioDevice *dev = new AudioDevice(i, friendly_name);
			this->devices.push_back(dev);
		}
		return this->devices;
	}

	bool AudioController::Start()
	{
		MMRESULT mr;
		WAVEHDR *buffer;
		for (int i = 0; i < buffer_count; ++i) {
			buffer = (WAVEHDR *) malloc(sizeof(WAVEHDR));
			buffer->dwBufferLength = buffer_size;
			buffer->lpData = new char[buffer->dwBufferLength];
			buffer->dwFlags = 0;
			mr = waveInPrepareHeader(wave_in_handle, buffer, sizeof(WAVEHDR));
			if (mr != MMSYSERR_NOERROR) {
				// TODO log it
				return false;
			}
			buffers.push_back(buffer);
			AddBuffer(buffer);
		}

		capturing = true;

		mr = waveInStart(wave_in_handle);
		return mr == MMSYSERR_NOERROR;
	}

	bool AudioController::Stop()
	{
		MMRESULT mr;
		mr = waveInStop(wave_in_handle);
		if (mr != MMSYSERR_NOERROR) {
			// TODO log it
		}
		Reset();

		mr = waveInClose(wave_in_handle);
		return mr == MMSYSERR_NOERROR;
	}

	void AudioController::Reset()
	{
		capturing = false;
		for (unsigned int i = 0; i < buffers.size(); ++i) {
			ReleaseBuffer(buffers[i]);
		}
	}

	void AudioController::ReleaseBuffer(WAVEHDR *buffer)
	{
		waveInUnprepareHeader(wave_in_handle, buffer, sizeof(WAVEHDR));
		delete[] buffer->lpData;
		free(buffer);
	}

	bool AudioController::SelectDevice(AudioDevice *device)
	{
		WAVEFORMATEX wave_format;
		wave_format.nChannels = channels;
		wave_format.nSamplesPerSec = samples_per_second;
		wave_format.wBitsPerSample = bits_per_sample;
		wave_format.nBlockAlign = wave_format.nChannels * (wave_format.wBitsPerSample / 8);
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;
		MMRESULT mr = waveInOpen(&wave_in_handle, device->DeviceId(), &wave_format, (DWORD_PTR) wave_in_proc, (DWORD_PTR) this, CALLBACK_FUNCTION);
		//mr = waveOutOpen(&wave_out_handle, WAVE_MAPPER, &wave_format, NULL, NULL, CALLBACK_NULL);
		return mr == MMSYSERR_NOERROR;
	}
}
