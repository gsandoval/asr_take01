
#include "audio_capture.h"
#include "online_feature_extractor.h"
#include "wave_recorder.h"
#include "raw_recorder.h"
#include "player.h"
#include "wave_reader.h"
#include "raw_reader.h"
#include "mfcc.h"
#include "training_set_reader.h"
#include "backpropagation_network.h"

#include <vector>
#include <iostream>

using namespace std;

const int channels = 1;
const int bits_per_sample = 16;
const int samples_per_second = 8000;

audiocapture::AudioController* CreateAudioController(int channels, int bits_per_sample, int samples_per_second)
{
	int buffer_size = (samples_per_second * channels * (bits_per_sample / 8)) << 1;
	audiocapture::AudioController *ac = new audiocapture::AudioController(channels, samples_per_second, bits_per_sample, buffer_size);
	return ac;
}

void PrintDevices(audiocapture::AudioController *ac)
{
	vector<audiocapture::AudioDevice*> devices = ac->ListDevices();
	for (unsigned int i = 0; i < devices.size(); ++i) {
		cout << devices[i]->FriendlyName() << "\n";
	}
}

int main(int argc, char* argv[])
{
	if (argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		cout << "ASR Take 1" << endl;
		cout << "Usage:" << endl;
		cout << "\t" << argv[0] << " -f\t\tTo recognize commands" << endl;
		cout << "\t" << argv[0] << " -p file.wav\tFor playing file.wav" << endl;
		cout << "\t" << argv[0] << " -r file.wav\tFor recording audio in file.wav" << endl;
		cout << "\t" << argv[0] << " -e file.wav\tFor extracting file.wav features" << endl;
		cout << "\t" << argv[0] << " -t file.wav file.raw\tFor converting file.wav into file.raw" << endl;
		return 0;
	}
	if (strcmp(argv[1], "-r") == 0) {
		if (argc < 3) {
			cout << "Missing parameter: file.ext" << endl;
			return -1;
		}
		string filename(argv[2]);
		audiocapture::AudioController *ac = CreateAudioController(channels, bits_per_sample, samples_per_second);
		asr_take01::Recorder *recorder;
		if (filename.rfind(".wav") == filename.size() - 4) {
			recorder = new asr_take01::WaveRecorder();
		} else if (filename.rfind(".raw") == filename.size() - 4) {
			recorder = new asr_take01::RawRecorder();
		}
		recorder->SetBitsPerSample(bits_per_sample);
		recorder->SetChannels(channels);
		recorder->SetSamplesPerSecond(samples_per_second);

		ac->AddRawAudioListener(recorder);
	
		//PrintDevices(ac);
		int selection;
		//cin >> selection;
		selection = 0;
		ac->SelectDevice(ac->ListDevices()[selection]);
	
		recorder->CreateOutputFile(filename);
		ac->Start();
		cout << "GO (enter a char to stop)" << endl;
		cin >> selection;
		recorder->CloseOutputFile();
		ac->Stop();

		delete ac;
		delete recorder;
	} else if (strcmp(argv[1], "-e") == 0) {
		if (argc < 3) {
			cout << "Missing parameter: file.ext" << endl;
			return -1;
		}
		string filename(argv[2]);
		asr_take01::FileReader *reader;
		if (filename.rfind(".wav") == filename.size() - 4) {
			reader = new asr_take01::WaveReader();
		} else if (filename.rfind(".raw") == filename.size() - 4) {
			reader = new asr_take01::RawReader();
			reader->SetChannels(channels);
			reader->SetBitsPerSample(bits_per_sample);
			reader->SetSamplesPerSecond(samples_per_second);
		}

		reader->OpenFile(filename);
		long long wave_size = reader->Size();
		char *wave_data = new char[(unsigned int) wave_size];
		reader->Read(wave_data, (unsigned int) wave_size);
		int channels = reader->Channels();
		int bits_per_sample = reader->BitsPerSample();
		int samples_per_second = reader->SamplesPerSecond();

		int total_features = 16;
		vector<float *> features;
		dsp::MFCC mfcc;
		mfcc.SetSamplingFrequency(samples_per_second);
		mfcc.SetCepstralCoefficientsNumber(total_features);
		mfcc.SetFrameLength(samples_per_second / 10 * 1); // 100 milliseconds
		mfcc.SetFrameShift(samples_per_second / 20 * 1);
		mfcc.SetFFTFrameLength(1024);
		//mfcc.SetFFTFrameLength(4096);
		//mfcc.SetFFTFrameLength(8192);
		mfcc.SetNoCoefficientZero(true);
		mfcc.Init();
		mfcc.Process(wave_data, (unsigned int) wave_size, features);
		cout.setf(ios::fixed,ios::floatfield);
		cout.precision(6);
		for (unsigned int i = 0; i < features.size(); ++i) {
			for (int j = 0; j < total_features; ++j) {
				cout << features[i][j];
				if (j != total_features - 1) {
					cout << "\t";
				}
			}
			cout << endl;

			delete[] features[i];
		}
		delete reader;
	} else if (strcmp(argv[1], "-p") == 0) {
		if (argc < 3) {
			cout << "Missing parameter: file.wav" << endl;
			return -1;
		}
		string filename(argv[2]);
		asr_take01::FileReader *reader;
		if (filename.rfind(".wav") == filename.size() - 4) {
			reader = new asr_take01::WaveReader();
		} else if (filename.rfind(".raw") == filename.size() - 4) {
			reader = new asr_take01::RawReader();
			reader->SetChannels(channels);
			reader->SetBitsPerSample(bits_per_sample);
			reader->SetSamplesPerSecond(samples_per_second);
		}

		reader->OpenFile(filename);
		long long wave_size = reader->Size();
		char *wave_data = new char[(unsigned int) wave_size];
		reader->Read(wave_data, (unsigned int) wave_size);
		int channels = reader->Channels();
		int bits_per_sample = reader->BitsPerSample();
		int samples_per_second = reader->SamplesPerSecond();
		reader->Close();

		asr_take01::Player player(channels, bits_per_sample, samples_per_second);
		player.Start();
		player.BlockRead(wave_data, (unsigned int) wave_size);
		player.Stop();

		delete reader;
	} else if (strcmp(argv[1], "-t") == 0) {
		if (argc < 4) {
			if (argc < 3) {
				cout << "Missing input parameter: file.wav" << endl;
			} else {
				cout << "Missing output parameter: file.raw" << endl;
			}
			return -1;
		}
		string input(argv[2]);
		string output(argv[3]);
		asr_take01::FileReader *reader;
		if (input.rfind(".wav") == input.size() - 4) {
			reader = new asr_take01::WaveReader();
		} else if (input.rfind(".raw") == input.size() - 4) {
			reader = new asr_take01::RawReader();
			reader->SetChannels(channels);
			reader->SetBitsPerSample(bits_per_sample);
			reader->SetSamplesPerSecond(samples_per_second);
		}
		asr_take01::Recorder *recorder;
		if (output.rfind(".wav") == output.size() - 4) {
			recorder = new asr_take01::WaveRecorder();
		} else if (output.rfind(".raw") == output.size() - 4) {
			recorder = new asr_take01::RawRecorder();
		}
		recorder->SetBitsPerSample(reader->BitsPerSample());
		recorder->SetChannels(reader->Channels());
		recorder->SetSamplesPerSecond(reader->SamplesPerSecond());

		reader->OpenFile(input);
		long long wave_size = reader->Size();
		char *wave_data = new char[(unsigned int) wave_size];
		reader->Read(wave_data, (unsigned int) wave_size);
		reader->Close();

		recorder->CreateOutputFile(output);
		recorder->BlockRead(wave_data, (int) wave_size);
		recorder->CloseOutputFile();

		delete reader;
		delete recorder;
	} else if (strcmp(argv[1], "-f") == 0) {
		asr_take01::OnlineFeatureExtractor listener(channels, bits_per_sample, samples_per_second);
		audiocapture::AudioController *ac = CreateAudioController(channels, bits_per_sample, samples_per_second);
		ac->AddRawAudioListener(&listener);
		
		PrintDevices(ac);
		int selection;
		cin >> selection;
		//selection = 0;
		ac->SelectDevice(ac->ListDevices()[selection]);
		ac->Start();
		//cin >> selection;
		ac->Stop();
		
		delete ac;
	} else if (strcmp(argv[1], "-train") == 0) {
		asr_take01::TrainingSetReader reader;
		reader.SetDirectory("..\\Debug\\features\\");
		vector<string> classes; classes.push_back("apagar"); classes.push_back("prender");
		reader.SetClassNames(classes);
		reader.SetSamplesPerClass(20);
		reader.Load();
		vector<vector<double> > apagar = reader.ClassSamples("apagar");
		vector<vector<double> > prender = reader.ClassSamples("prender");
		int features_per_sample = 11 * 16;
		
		// Normalize them all
		double max_value = 0;
		for (unsigned int i = 0; i < apagar.size(); ++i) {
			for (unsigned int j = 0; j < features_per_sample; ++j) {
				if (max_value < apagar[i][j]) {
					max_value = apagar[i][j];
				}
				if (max_value < prender[i][j]) {
					max_value = prender[i][j];
				}
			}
		}
		for (unsigned int i = 0; i < apagar.size(); ++i) {
			for (unsigned int j = 0; j < features_per_sample; ++j) {
				apagar[i][j] /= max_value;
				prender[i][j] /= max_value;
			}
		}

		vector<int> layers; layers.push_back(176); layers.push_back(90); layers.push_back(45); layers.push_back(3);
		softcomputing::BackPropagationNetwork nn(layers);
		nn.SetLearningRate(0.001);
		nn.SetMaxEpochs(10000);
		nn.SetMaxError(0.1);
		nn.SetMomentum(0.9);
		//nn.SetQuiet(true);

		vector<vector<double> > training_set;
		vector<vector<double> > expected;
		vector<double> expected_apagar; expected_apagar.push_back(0); expected_apagar.push_back(1); expected_apagar.push_back(0);
		vector<double> expected_prender; expected_prender.push_back(0); expected_prender.push_back(0); expected_prender.push_back(1);
		for (unsigned int i = 0; i < apagar.size(); ++i) {
			if (static_cast<int>(apagar[i].size()) < layers[0]) {
				for (int j = apagar[i].size(); j < layers[0]; ++j) {
					apagar[i].push_back(0);
				}
			}
			training_set.push_back(apagar[i]);
			if (static_cast<int>(prender[i].size()) < layers[0]) {
				for (int j = prender[i].size(); j < layers[0]; ++j) {
					prender[i].push_back(0);
				}
			}
			training_set.push_back(prender[i]);

			expected.push_back(expected_apagar);
			expected.push_back(expected_prender);
		}
		
		nn.Train(training_set, expected);

		for (unsigned int i = 0; i < training_set.size(); ++i) {
			vector<double> output = nn.Classify(training_set[i]);
			cerr << "(";
			for (unsigned int j = 0; j < output.size(); ++j) {
				cerr << output[j];
				if (j != output.size() - 1) {
					cerr << ", ";
				}
			}
			cerr << ") = (";
			for (unsigned int j = 0; j < expected[i].size(); ++j) {
				cerr << static_cast<int>(expected[i][j]);
				if (j != expected[i].size() - 1) {
					cerr << ", ";
				}
			}
			cerr << ")" << endl;
		}

		vector<vector<vector<double> > > weights = nn.Weights();
		vector<softcomputing::ActivationFunction> activation_functions = nn.ActivationFunctions();

		/*
		* Print network config
		*/
		cout << layers.size() << endl;
		for (unsigned int i = 0; i < layers.size(); ++i) {
			cout << layers[i] << endl;
		}
		for (unsigned int i = 0; i < activation_functions.size(); ++i) {
			cout << activation_functions[i] << endl;
		}
		for (unsigned int i = 0; i < weights.size(); ++i) {
			for (unsigned int j = 0; j < weights[i].size(); ++j) {
				for (unsigned int k = 0; k < weights[i][j].size(); ++k) {
					cout << weights[i][j][k];
					if (k != weights[i][j].size() - 1) {
						cout << "\t";
					}
				}
				cout << endl;
			}
		}
	}
	return 0;
}
