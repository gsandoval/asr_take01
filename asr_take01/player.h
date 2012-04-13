#ifndef PLAYER_H_
#define PLAYER_H_

#include "raw_audio_listener.h"
#include <Windows.h>

namespace asr_take01
{
	void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);

	class Player : public audiocapture::RawAudioListener
	{
	private:
		HWAVEOUT wave_out_handle;
		int channels;
		int bits_per_sample;
		int samples_per_second;
		bool device_opened;
		int buffer_playing_count;
	public:
		Player(int channels, int bits_per_sample, int samples_per_second);
		virtual ~Player();
		void BlockRead(char *block, int block_size);
		void Start();
		void Stop();
		friend void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT msg, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2);
	};
}

#endif
