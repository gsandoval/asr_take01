
#include "command_recognizer.h"

#include "backpropagation_network.h"
#include <iostream>

namespace asr_take01
{
	using namespace std;

	CommandRecognizer::CommandRecognizer(softcomputing::BackPropagationNetwork *nn)
	{
		this->nn = nn;
	}

	CommandRecognizer::~CommandRecognizer()
	{
	}

	void CommandRecognizer::FeatureReady(vector<double> feature)
	{
		double max_value = 0;
		for (unsigned int i = 0; i < feature.size(); ++i) {
			if (max_value < feature[i]) {
				max_value = feature[i];
			}
		}
		for (unsigned int i = 0; i < feature.size(); ++i) {
			feature[i] /= 28.5;
		}
		vector<double> output = nn->Classify(feature);
		vector<int> command;
		for (unsigned int i = 0; i < output.size(); ++i) {
			command.push_back(static_cast<int>(output[i] + 0.1));
		}

		if (command.size() >= 3) {
			if (command[0] == 1 && command[1] == 0 && command[2] == 0) {
				cout << "apagar" << endl;
			} else if (command[0] == 0 && command[1] == 1 && command[2] == 0) {
				cout << "prender" << endl;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 1) {
				//cout << "silence" << endl;
			}
		}
	}
}