
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

	bool CommandRecognizer::FeatureReady(vector<double> feature)
	{
		for (unsigned int i = 0; i < feature.size(); ++i) {
			feature[i] /= 30;
		}
		vector<double> output = nn->Classify(feature);
		vector<int> command;
		for (unsigned int i = 0; i < output.size(); ++i) {
			command.push_back(static_cast<int>(output[i] + 0.05));
		}

		bool found_something = false;
		if (command.size() == 3) {
			if (command[0] == 1 && command[1] == 0 && command[2] == 0) {
				cout << "apagar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 1 && command[2] == 0) {
				cout << "prender" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 1) {
				//cout << "silence" << endl;
			}
		} else if (command.size() == 4) {
			if (command[0] == 1 && command[1] == 0 && command[2] == 0 && command[3] == 0) {
				cout << "apagar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 1 && command[2] == 0 && command[3] == 0) {
				cout << "prender" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 1 && command[3] == 0) {
				cout << "luces" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 1) {
				//cout << "silence" << endl;
			}
		}
		return found_something;
	}
}