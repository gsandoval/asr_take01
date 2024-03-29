
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
#include "command_recognizer.h"

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

const int channels = 1;
const int bits_per_sample = 16;
const int samples_per_second = 8000;
const int mel_filter_bank_size = 23;
const int fft_length = 4096;
const int frame_length = samples_per_second / 10 * 4;
const int frame_shift = samples_per_second / 20 * 2;
const int cepstrum_coefficients = 16;
const int vectors_per_sample = 3;

audiocapture::AudioController* CreateAudioController(int channels, int bits_per_sample, int samples_per_second)
{
	int buffer_size = (samples_per_second * channels * (bits_per_sample / 8)) << 1;
	audiocapture::AudioController *ac = new audiocapture::AudioController(channels, samples_per_second, bits_per_sample, 5, buffer_size);
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
	
		int selection;
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

		vector<float *> features;
		dsp::MFCC mfcc;
		mfcc.SetSamplingFrequency(samples_per_second);
		mfcc.SetCepstralCoefficientsNumber(cepstrum_coefficients);
		mfcc.SetFrameLength(frame_length);
		mfcc.SetFrameShift(frame_shift);
		mfcc.SetMelFilterBankSize(mel_filter_bank_size);
		mfcc.SetFFTFrameLength(fft_length);
		mfcc.SetNoCoefficientZero(true);
		mfcc.Init();
		mfcc.Process(wave_data, (unsigned int) wave_size, features);
		cout.setf(ios::fixed,ios::floatfield);
		cout.precision(6);
		for (unsigned int i = 0; i < features.size(); ++i) {
			for (int j = 0; j < cepstrum_coefficients; ++j) {
				cout << features[i][j];
				if (j != cepstrum_coefficients - 1) {
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
		if (argc < 3) {
			cout << "Missing input parameter: network_definition.txt" << endl;
			return -1;
		}
		ifstream file(argv[2], ios::in);
		audiocapture::AudioController *ac = CreateAudioController(channels, bits_per_sample, samples_per_second);
		asr_take01::OnlineFeatureExtractor feature_extractor(cepstrum_coefficients, channels, bits_per_sample, samples_per_second, 
			ac->BufferSize(), fft_length, mel_filter_bank_size, frame_length, frame_shift, vectors_per_sample);
		ac->AddRawAudioListener(&feature_extractor);
		
		int layer_count;
		file >> layer_count;
		vector<int> layers;
		int layer_size;
		for (int i = 0; i < layer_count; ++i) {
			file >> layer_size;
			layers.push_back(layer_size);
		}
		vector<softcomputing::ActivationFunction> activation_functions;
		int activation_function;
		for (int i = 0; i < layer_count - 1; ++i) {
			file >> activation_function;
			activation_functions.push_back((softcomputing::ActivationFunction) activation_function);
		}
		vector<vector<vector<double> > > weights;
		double tmp;
		for (unsigned int i = 1; i < layers.size(); ++i) {
			vector<vector<double> > layer;
			for (int j = 0; j < layers[i]; ++j) {
				vector<double> neuron;
				for (int k = 0; k < layers[i - 1]; ++k) {
					file >> tmp;
					neuron.push_back(tmp);
				}
				layer.push_back(neuron);
			}
			weights.push_back(layer);
		}

		softcomputing::BackPropagationNetwork nn(layers, 1, activation_functions);
		nn.SetWeights(weights);

		vector<string> output_classes;
		vector<vector<double> > output_values;
		string class_name;
		double vector_val;
		for (int i = 0; i < layers[layers.size() - 1]; ++i) {
			file >> class_name;
			output_classes.push_back(class_name);
			vector<double> class_vector;
			for (int j = 0; j < layers[layers.size() - 1]; ++j) {
				file >> vector_val;
				class_vector.push_back(vector_val);
			}
			output_values.push_back(class_vector);
		}

		asr_take01::CommandRecognizer *feature_listener = new asr_take01::CommandRecognizer(&nn);
		feature_listener->SetOutputClasses(output_classes, output_values);
		feature_extractor.AddFeatureListener(feature_listener);
		
		int selection;
		selection = 0;
		ac->SelectDevice(ac->ListDevices()[selection]);
		ac->Start();
		cin >> selection;
		ac->Stop();
		
		delete ac;
		delete feature_listener;
	} else if (strcmp(argv[1], "-train") == 0) {
		if (argc < 4) {
			if (argc < 3) {
				cout << "Missing input directory: .\\features_directory\\" << endl;
			} else {
				cout << "Missing at least one class" << endl;
			}
			return -1;
		}
		asr_take01::TrainingSetReader reader;
		reader.SetDirectory(argv[2]);
		vector<string> classes;
		for (int i = 3; i < argc; ++i) {
			classes.push_back(argv[i]);
		}
		reader.SetClassNames(classes);
		int samples_per_class = 20;
		reader.SetSamplesPerClass(samples_per_class);
		reader.Load();

		vector<vector<vector<double> > > class_samples;
		for (unsigned int i = 0; i < classes.size(); ++i) {
			class_samples.push_back(reader.ClassSamples(classes[i]));
		}
		int features_per_sample = vectors_per_sample * cepstrum_coefficients;
		
		// Normalize them all
		double max_value = 30;
		for (unsigned int k = 0; k < class_samples.size(); ++k) {
			for (int i = 0; i < samples_per_class; ++i) {
				for (int j = 0; j < features_per_sample; ++j) {
					class_samples[k][i][j] /= max_value;
				}
			}
		}

		// 90, 45
		vector<int> layers; 
		layers.push_back(features_per_sample); 
		layers.push_back((features_per_sample + classes.size()) / 2); 
		layers.push_back(layers[1] / 2);
		layers.push_back(classes.size());
		
		softcomputing::BackPropagationNetwork nn(layers);
		nn.SetLearningRate(0.1);
		nn.SetMaxEpochs(1000000);
		nn.SetMaxError(0.0001);
		nn.SetMomentum(0.9);
		//nn.SetQuiet(true);

		vector<vector<double> > training_set;
		vector<vector<double> > expected;
		for (int i = 0; i < samples_per_class; ++i) {
			for (unsigned int k = 0; k < class_samples.size(); ++k) {
				training_set.push_back(class_samples[k][i]);
				vector<double> expected_output;
				for (int j = 0; j < layers[layers.size() - 1]; ++j) {
					expected_output.push_back(k == j? 1 : 0);
				}
				expected.push_back(expected_output);
			}
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

		for (unsigned int i = 0; i < classes.size(); ++i) {
			cout << classes[i];
			for (int j = 0; j < layers[layers.size() - 1]; ++j) {
				cout << " " << (i == j? 1 : 0);
			}
			cout << endl;
		}
	}
	return 0;
}
