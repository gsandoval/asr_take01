
#ifndef COMMAND_RECOGNIZER_H_
#define COMMAND_RECOGNIZER_H_

#include "feature_listener.h"

#include <vector>
#include <string>

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
		std::vector<std::string> class_names;
		std::vector<std::vector<double> > class_values;
	public:
		CommandRecognizer(softcomputing::BackPropagationNetwork *nn);
		virtual ~CommandRecognizer();
		bool FeatureReady(std::vector<double> feature);
		void SetOutputClasses(std::vector<std::string> class_names, std::vector<std::vector<double> > class_values);
	};
}

#endif
