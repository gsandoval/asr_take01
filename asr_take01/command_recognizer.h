
#ifndef COMMAND_RECOGNIZER_H_
#define COMMAND_RECOGNIZER_H_

#include "feature_listener.h"

#include <vector>

namespace softcomputing
{
	class BackPropagationNetwork;
}

namespace asr_take01
{
	class CommandRecognizer : public FeatureListener
	{
	private:
		softcomputing::BackPropagationNetwork *nn;
	public:
		CommandRecognizer(softcomputing::BackPropagationNetwork *nn);
		virtual ~CommandRecognizer();
		void FeatureReady(std::vector<double> feature);
	};
}

#endif
