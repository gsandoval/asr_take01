
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
		bool no_log_e, no_c0;
		bool swap_specified;
		float starting_frequency;
		float energy_floor_fb, energy_floor_log_e;
		int frame_length, frame_shift, fft_length;
		float *circular_float_buffer;
		int cfb_size, cfb_pointer;
		float float_buffer[MAX_WINDOW_SIZE + 1];
		float float_window[MAX_WINDOW_SIZE / 2];
		float *dct_matrix;
		FFT_Window_tag *first_window;
		int ReadBlock(float *block, int block_pos, int block_size, float *float_buffer, int buffer_size, 
			int buffer_pointer, int samples);
	public:
		MFCC();
		~MFCC();
		void Process(char *block, int block_size, std::vector<float *> &cep_coeffs);
	};
}

#endif
