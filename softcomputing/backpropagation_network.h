
#ifndef BACK_PROPAGATION_NETWORK_H_
#define BACK_PROPAGATION_NETWORK_H_

#include <vector>

namespace softcomputing
{
	enum ActivationFunction
	{
		Sigmoid, Tangent, Linear, Step
	};

	class BackPropagationNetwork
	{
	private:
		class Neuron
		{
		private:
			double input;
			double output;
			double output_derivative;
			std::vector<double> weights;
			double delta;
			ActivationFunction activation_function;
			double last_weight_update;
		public:
			Neuron(ActivationFunction activation_function);
			virtual ~Neuron();
			void InitWeights(int prev_layer_size, double weight_init_magnitud);
			void ForceOutput(double output);
			double Output();
			double OutputDerivative();
			void SetWeight(int n_prev_layer, double weight);
			double Weight(int n_prev_layer);
			void SetInput(double input);
			void Fire();
			double Delta();
			void SetDelta(double delta);
			std::vector<double> Weights();
			double LastWeightUpdate();
			void SetLastWeightUpdate(double weight_update);
		};
	public:
		BackPropagationNetwork(std::vector<int> layer_sizes, 
			double weight_init_magnitud = 1,
			std::vector<ActivationFunction> activation_functions = std::vector<ActivationFunction>());
		virtual ~BackPropagationNetwork();
		void Train(std::vector<std::vector<double> > training_set, std::vector<std::vector<double> > expected);
		std::vector<double> Classify(std::vector<double> data);
		std::vector<std::vector<std::vector<double> > > Weights();
		void SetQuiet(bool quiet);
		std::vector<ActivationFunction> ActivationFunctions();
		void SetMaxError(double max_error);
		void SetMaxEpochs(int max_epochs);
		void SetLearningRate(double learning_rate);
		void SetMomentum(double momentum);
		void SetWeights(std::vector<std::vector<std::vector<double> > > weights);
	private:
		std::vector<std::vector<Neuron*> > layers;
		std::vector<ActivationFunction> activation_functions;
		double max_error;
		int max_epochs;
		double learning_rate;
		bool quiet;
		double momentum;
		void BackPropagate(std::vector<double> err);
	};
}

#endif
