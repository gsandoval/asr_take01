
#include "training_set_reader.h"

#include <sstream>
#include <fstream>
#include <iostream>

namespace asr_take01
{
	using namespace std;

	TrainingSetReader::TrainingSetReader()
	{
	}

	TrainingSetReader::~TrainingSetReader()
	{
	}

	void TrainingSetReader::Load()
	{
		ostringstream oss;
		const int line_size = 1024;
		char line[line_size];
		double elem;
		for (unsigned int i = 0; i < classes.size(); ++i) {
			for (int j = 1; j <= samples_per_class; ++j) {
				oss = ostringstream();
				oss << path << classes[i];
				if (j > 9) {
					oss << j;
				} else {
					oss << 0 << j;
				}
				oss << ".txt";
				string filename = oss.str();
				ifstream file(filename, ios::in);
				vector<double> sample;
				while (file.good()) {
					file >> elem;
					sample.push_back(elem);
				}
				samples[classes[i]].push_back(sample);
			}
		}
	}

	vector<vector<double> > TrainingSetReader::ClassSamples(string clazz)
	{
		return samples[clazz];
	}

	void TrainingSetReader::SetDirectory(string path)
	{
		this->path = path;
	}

	void TrainingSetReader::SetClassNames(vector<string> classes)
	{
		this->classes = classes;
	}

	void TrainingSetReader::SetSamplesPerClass(int samples_per_class)
	{
		this->samples_per_class = samples_per_class;
	}
}
