
#ifndef FEATURE_LISTENER_H_
#define FEATURE_LISTENER_H_

#include <vector>

namespace asr_take01
{
	class FeatureListener
	{
	public:
		virtual ~FeatureListener() {}
		virtual bool FeatureReady(std::vector<double> feature) = 0;
	};
}

#endif
