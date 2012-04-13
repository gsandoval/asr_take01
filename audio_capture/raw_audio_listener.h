
#ifndef RAW_AUDIO_LISTENER_H_
#define RAW_AUDIO_LISTENER_H_

#include <vector>

namespace audiocapture
{
	class RawAudioListener
	{
	public:
		virtual ~RawAudioListener()
		{
		}
		virtual void BlockRead(char *block, int block_size) = 0;
	};
}

#endif
