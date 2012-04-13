
#include "audio_device.h"

namespace audiocapture
{
	AudioDevice::AudioDevice(int _device_id, std::string _friendly_name) 
		: friendly_name(_friendly_name), device_id(_device_id)
	{
	}

	AudioDevice::~AudioDevice()
	{
	}

	std::string AudioDevice::FriendlyName()
	{
		return friendly_name;
	}

	int AudioDevice::DeviceId()
	{
		return device_id;
	}
}
