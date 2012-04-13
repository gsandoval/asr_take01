
#include "backpropagation_network.h"

#include <ctime>
#include <cstdlib>
#include <limits>
#include <iostream>

namespace softcomputing
{
	using namespace std;

	BackPropagationNetwork::Neuron::Neuron(ActivationFunction _activation_function) : activation_function(_activation_function)
	{

	}

	BackPropagationNetwork::Neuron::~Neuron()
	{

	}

	void BackPropagationNetwork::Neuron::InitWeights(int prev_layer_size, double weight_init_magnitud)
	{
		for (int i = 0; i < prev_layer_size; ++i) {
			//double w = rand() % RAND_MAX * 1.0 / RAND_MAX * 2 - 1;
			double w = rand() % RAND_MAX * 1.0 / RAND_MAX * 2 - 1;
			w *= weight_init_magnitud;
			weights.push_back(w);
		}
	}

	void BackPropagationNetwork::Neuron::ForceOutput(double output)
	{
		this->output = output;
	}

	double BackPropagationNetwork::Neuron::Weight(int n_prev_layer)
	{
		return weights[n_prev_layer];
	}

	void BackPropagationNetwork::Neuron::SetWeight(int n_prev_layer, double weight)
	{
		weights[n_prev_layer] = weight;
	}

	double BackPropagationNetwork::Neuron::OutputDerivative()
	{
		return output_derivative;
	}

	double BackPropagationNetwork::Neuron::Output()
	{
		return output;
	}

	void BackPropagationNetwork::Neuron::SetInput(double input)
	{
		this->input = input;
	}

	void BackPropagationNetwork::Neuron::SetDelta(double delta)
	{
		this->delta = delta;
	}

	double BackPropagationNetwork::Neuron::Delta()
	{
		return delta;
	}

	vector<double> BackPropagationNetwork::Neuron::Weights()
	{
		return weights;
	}

	void BackPropagationNetwork::Neuron::Fire()
	{
		if (activation_function == Linear) {
			output = input;
			output_derivative = 1;
		} else if (activation_function == Sigmoid) {
			output = 1.0 / (1 + exp(-1 * input));
			output_derivative = output * (1 - output);
		} else if (activation_function == Tangent) {
			output = (exp(input * 2) - 1) / (exp(input * 2) + 1);
			output_derivative = 1 - pow(output, 2);
		} else if (activation_function == Step) {
			output = input >= 0? 1 : 0;
			output_derivative = 1;
		}
	}

	BackPropagationNetwork::BackPropagationNetwork(double _max_error, long long _max_epochs, double _learning_rate, 
		vector<int> layer_sizes, double weight_init_magnitud, vector<ActivationFunction> activation_functions) 
		: max_error(_max_error), max_epochs(_max_epochs),  learning_rate(_learning_rate), quiet(false)
	{
		srand((unsigned int) time(NULL));
		
		int activation_function_size = (int) activation_functions.size();
		for (unsigned int i = 0; i < layer_sizes.size(); ++i) {
			vector<Neuron*> layer;
			ActivationFunction activation_function = Sigmoid;
			if (i != 0 && i <= activation_function_size - 1 && activation_function_size > 0) {
				activation_function = activation_functions[i - 1];
			}
			if (i != 0) {
				this->activation_functions.push_back(activation_function);
			}
			for (int j = 0; j < layer_sizes[i]; ++j) {
				Neuron *n = new Neuron(activation_function);
				layer.push_back(n);
				if (i != 0) {
					n->InitWeights(layer_sizes[i - 1], weight_init_magnitud);
				}
			}
			layers.push_back(layer);
		}
	}

	vector<ActivationFunction> BackPropagationNetwork::ActivationFunctions()
	{
		return activation_functions;
	}

	BackPropagationNetwork::~BackPropagationNetwork()
	{

	}

	void BackPropagationNetwork::SetQuiet(bool quiet)
	{
		this->quiet = quiet;
	}

	vector<vector<vector<double> > > BackPropagationNetwork::Weights()
	{
		vector<vector<vector<double> > > weights;
		for (unsigned int i = 1; i < layers.size(); ++i) {
			vector<vector<double> > layer_weights;
			for (unsigned int j = 0; j < layers[i].size(); ++j) {
				layer_weights.push_back(layers[i][j]->Weights());
			}
			weights.push_back(layer_weights);
		}
		return weights;
	}

	void BackPropagationNetwork::BackPropagate(vector<double> err)
	{
		for (unsigned int i = 0; i < layers[layers.size() - 1].size(); ++i) {
			layers[layers.size() - 1][i]->SetDelta(err[i]);
		}
		for (unsigned int i = layers.size() - 2; i > 0; --i) {
			for (unsigned int j = 0; j < layers[i].size(); ++j) {
				Neuron *n = layers[i][j];
				double delta = 0;
				for (unsigned int k = 0; k < layers[i + 1].size(); ++k) {
					Neuron *next_n = layers[i + 1][k];
					delta += next_n->Delta() * next_n->Weight(j);
				}
				n->SetDelta(delta);
			}
		}
		for (unsigned int i = 1; i < layers.size(); ++i) {
			for (unsigned int j = 0; j < layers[i].size(); ++j) {
				Neuron *n = layers[i][j];
				for (unsigned int k = 0; k < layers[i - 1].size(); ++k) {
					Neuron *prev_n = layers[i - 1][k];
					n->SetWeight(k, n->Weight(k) + learning_rate * n->Delta() * n->OutputDerivative() * prev_n->Output());
				}
			}
		}
	}

	void BackPropagationNetwork::Train(vector<vector<double> > training_set, vector<vector<double> > expected)
	{
		double epoch_error = 0;
		for (int i = 0; i < max_epochs; ++i) {
			epoch_error = 0;
			for (unsigned int j = 0; j < training_set.size(); ++j) {
				vector<double> output = Classify(training_set[j]);
				vector<double> error(output.size());
				for (unsigned int k = 0; k < output.size(); ++k) {
					error[k] = expected[j][k] - output[k];
					epoch_error = epoch_error + pow(error[k], 2);
				}
				BackPropagate(error);
			}
			epoch_error = epoch_error / 2;
			if (!quiet) {
				cerr << "epoch " << i << ": " << epoch_error << endl;
			}
			if (epoch_error < max_error) {
				break;
			}
		}
	}

	vector<double> BackPropagationNetwork::Classify(vector<double> pattern)
	{
		vector<double> output;
		for (unsigned int i = 0; i < layers[0].size(); ++i) {
			layers[0][i]->ForceOutput(pattern[i]);
		}
		for (unsigned int i = 1; i < layers.size(); ++i) {
			for (unsigned int j = 0; j < layers[i].size(); ++j) {
				Neuron *n = layers[i][j];
				double input = 0;
				for (unsigned int k = 0; k < layers[i - 1].size(); ++k) {
					Neuron *prev_n = layers[i - 1][k];
					input += prev_n->Output() * n->Weight(k);
				}
				n->SetInput(input);
				n->Fire();

				if (i == layers.size() - 1) {
					//output.push_back(input > 0 ? 1.0 : 0.0);
					output.push_back(n->Output());
				}
			}
		}
		return output;
	}
}
