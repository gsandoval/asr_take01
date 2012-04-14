
#include "backpropagation_network.h"

#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	vector<int> layer_sizes;
	layer_sizes.push_back(3); layer_sizes.push_back(5); layer_sizes.push_back(3); layer_sizes.push_back(1);

	vector<vector<double> > training_set;
	vector<double> t1; t1.push_back(0); t1.push_back(0); t1.push_back(0);
	vector<double> t2; t2.push_back(0); t2.push_back(0); t2.push_back(1);
	vector<double> t3; t3.push_back(0); t3.push_back(1); t3.push_back(0);
	vector<double> t4; t4.push_back(0); t4.push_back(1); t4.push_back(1);
	vector<double> t5; t5.push_back(1); t5.push_back(0); t5.push_back(0);
	vector<double> t6; t6.push_back(1); t6.push_back(0); t6.push_back(1);
	vector<double> t7; t7.push_back(1); t7.push_back(1); t7.push_back(0);
	vector<double> t8; t8.push_back(1); t8.push_back(1); t8.push_back(1);
	training_set.push_back(t1); training_set.push_back(t2); training_set.push_back(t3);
	training_set.push_back(t4); training_set.push_back(t5); training_set.push_back(t6);
	training_set.push_back(t7); training_set.push_back(t8);
	vector<vector<double> > expected_output;
	vector<double> o1; o1.push_back(0); vector<double> o2; o2.push_back(1);
	vector<double> o3; o3.push_back(1); vector<double> o4; o4.push_back(0);
	vector<double> o5; o5.push_back(1); vector<double> o6; o6.push_back(0);
	vector<double> o7; o7.push_back(0); vector<double> o8; o8.push_back(1);
	expected_output.push_back(o1); expected_output.push_back(o2); expected_output.push_back(o3);
	expected_output.push_back(o4); expected_output.push_back(o5); expected_output.push_back(o6);
	expected_output.push_back(o7); expected_output.push_back(o8);

	softcomputing::BackPropagationNetwork nn(layer_sizes);
	nn.SetLearningRate(0.3);
	nn.SetMaxEpochs(100000);
	nn.SetMaxError(0.001);
	nn.Train(training_set, expected_output);

	for (unsigned int i = 0; i < training_set.size(); ++i) {
		vector<double> output = nn.Classify(training_set[i]);
		cout << "(" << training_set[i][0] << ", " << training_set[i][1] << ", " << training_set[i][2] << ") = " << static_cast<int>(output[0] + 0.5) << endl;
	}
	char wait_input;
	cin >> wait_input;
	return 0;
}
