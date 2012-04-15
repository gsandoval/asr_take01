
#ifndef MFCC_H_
#define MFCC_H_

#include <vector>

struct FFT_Window_tag;

namespace dsp
{
	#define MAX_WINDOW_SIZE				512

	class MFCC
	{
	private:
		int native_byte_order;
		int sampling_frequency;
		bool no_log_energy, no_coefficient_zero;
		bool swap_specified;
		float starting_frequency;
		float energy_floor_fb, energy_floor_log_e;
		int frame_length, frame_shift, fft_length;
		float *circular_float_buffer;
		int cfb_size, cfb_pointer;
		float *float_buffer;
		float *float_window;
		float *dct_matrix;
		FFT_Window_tag *first_window;
		int cepstral_coefficient_number;
		int ReadBlock(float *block, int block_pos, int block_size, float *float_buffer, int buffer_size, 
			int buffer_pointer, int samples);
	public:
		MFCC();
		~MFCC();
		void Init();
		void SetSamplingFrequency(int sampling_frequency);
		void SetFrameShift(int frame_shift);
		void SetFrameLength(int window_size);
		void SetFFTFrameLength(int window_size);
		void SetCepstralCoefficientsNumber(int cepstral_coefficient_number);
		void SetNoCoefficientZero(bool no_coefficient_zero);
		void SetNoLogEnergy(bool no_log_energy);
		void Process(char *block, int block_size, std::vector<float *> &cep_coeffs);
		//void Process(Buffer<char> *buffer, std::vector<float *> &cep_coeffs);
	};
}

#endif
