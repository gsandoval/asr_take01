
#ifndef AUDIO_DEVICE_H_
#define AUDIO_DEVICE_H_

#include <string>

namespace audiocapture
{
	class AudioDevice
	{
	public:
		AudioDevice(int device_id, std::string friendly_name);
		virtual ~AudioDevice();
		std::string FriendlyName();
		int DeviceId();
	private:
		std::string friendly_name;
		int device_id;
	};
}

#endif
