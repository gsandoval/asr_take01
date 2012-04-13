
#ifndef TRAINING_SET_READER_H_
#define TRAINING_SET_READER_H_

#include <string>
#include <vector>
#include <map>

namespace asr_take01
{
	class TrainingSetReader
	{
	private:
		std::string path;
		int samples_per_class;
		std::vector<std::string> classes;
		std::map<std::string, std::vector<std::vector<double> > > samples;
	public:
		TrainingSetReader();
		virtual ~TrainingSetReader();
		void SetDirectory(std::string path);
		void SetClassNames(std::vector<std::string> classes);
		void SetSamplesPerClass(int samples_per_class);
		std::vector<std::vector<double> > ClassSamples(std::string clazz);
		void Load();
	};
}

#endif
