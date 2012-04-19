
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

	void CommandRecognizer::SetOutputClasses(vector<string> class_names, vector<vector<double> > class_values)
	{
		this->class_names = class_names;
		this->class_values = class_values;
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

		for (unsigned int i = 0; i < class_names.size(); ++i) {
			bool match = true;
			for (unsigned int j = 0; j < command.size(); ++j) {
				if (command[j] != class_values[i][j]) {
					match = false;
					break;
				}
			}
			if (match) {
				found_something = true;
				cout << class_names[i] << endl;
				break;
			}
		}

		/*
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
		} else if (command.size() == 6) {
			if (command[0] == 1 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 0) {
				cout << "apagar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 1 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 0) {
				cout << "prender" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 1 && command[3] == 0 && command[4] == 0 && command[5] == 0) {
				cout << "abrir" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 1 && command[4] == 0 && command[5] == 0) {
				cout << "cerrar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 1 && command[5] == 0) {
				cout << "puerta" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 1) {
				cout << "luces" << endl;
				found_something = true;
			}
		} else if (command.size() == 7) {
			if (command[0] == 1 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 0 && command[6]) {
				cout << "apagar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 1 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 0 && command[6] == 0) {
				cout << "prender" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 1 && command[3] == 0 && command[4] == 0 && command[5] == 0 && command[6] == 0) {
				cout << "abrir" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 1 && command[4] == 0 && command[5] == 0 && command[6] == 0) {
				cout << "cerrar" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 1 && command[5] == 0 && command[6] == 0) {
				cout << "puerta" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 1 && command[6] == 0) {
				cout << "luces" << endl;
				found_something = true;
			} else if (command[0] == 0 && command[1] == 0 && command[2] == 0 && command[3] == 0 && command[4] == 0 && command[5] == 0 && command[6] == 1) {
				//cout << "luces" << endl;
				//found_something = true;
			}
		}
		*/
		return found_something;
	}
}