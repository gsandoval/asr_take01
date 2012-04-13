
#include "player.h"

#include <iostream>

namespace asr_take01
{
	void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2)
	{
		Player *player = reinterpret_cast<Player *>(instance);
		WAVEHDR *buffer;
		switch (msg) {
		case WOM_CLOSE:
			break;
		case WOM_DONE:
			player->buffer_playing_count--;
			buffer = (WAVEHDR *) param1;
			free(buffer);
			break;
		case WOM_OPEN:
			break;
		}
	}

	Player::Player(int _channels, int _bits_per_sample, int _samples_per_second) : channels(_channels), 
		bits_per_sample(_bits_per_sample), samples_per_second(_samples_per_second), device_opened(false),
		buffer_playing_count(0)
	{
	}

	Player::~Player()
	{
	}

	void Player::BlockRead(char *block, int block_size)
	{
		buffer_playing_count++;
		WAVEHDR *buffer = (WAVEHDR *) malloc(sizeof(WAVEHDR));
		buffer->lpData = block;
		buffer->dwBufferLength = block_size;
		buffer->dwBytesRecorded = block_size;
		buffer->dwFlags = 0;
		waveOutPrepareHeader(wave_out_handle, buffer, sizeof(WAVEHDR));
		MMRESULT mr = waveOutWrite(wave_out_handle, buffer, sizeof(WAVEHDR));
	}

	void Player::Start()
	{
		WAVEFORMATEX wave_format;
		wave_format.nChannels = channels;
		wave_format.nSamplesPerSec = samples_per_second;
		wave_format.wBitsPerSample = bits_per_sample;
		wave_format.nBlockAlign = wave_format.nChannels * (wave_format.wBitsPerSample / 8);
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;
		MMRESULT mr = waveOutOpen(&wave_out_handle, WAVE_MAPPER, &wave_format, (DWORD_PTR) wave_out_proc, (DWORD_PTR) this, CALLBACK_FUNCTION);
		device_opened = true;
	}

	void Player::Stop()
	{
		while (buffer_playing_count > 0) {
			Sleep(100);
		}
		device_opened = false;
		waveOutClose(wave_out_handle);
	}
}
