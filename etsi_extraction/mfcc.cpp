
#include "mfcc.h"

extern "C" {
#include "FEfunc.h"
}

#include <cmath>
#include <cstdio>
#include <cstdlib>

#define ENERGYFLOOR_FB				-50.0  /*0.0 */
#define ENERGYFLOOR_logE			-50.0  /*4.0 */

#define IEEE_LE						0
#define IEEE_BE						1

#define SAMPLING_FREQ_1				8  /*8kHz */
#define SAMPLING_FREQ_2				11  /*11kHz */
#define SAMPLING_FREQ_3				16  /*16kHz */

#define FRAME_LENGTH_1				200  /*25ms */
#define FRAME_LENGTH_2				256  /*23.27ms */
#define FRAME_LENGTH_3				400  /*25ms */

#define FRAME_SHIFT_1				80  /*10ms */
#define FRAME_SHIFT_2				110  /*10ms */
#define FRAME_SHIFT_3				160  /*10ms */

#define PRE_EMPHASIS				0.97

#define FFT_LENGTH_1				256
#define FFT_LENGTH_2				256
#define FFT_LENGTH_3				512

#define NUM_CHANNELS				23
#define STARTING_FREQ_1				64.0  /*55.401825 */
#define STARTING_FREQ_2				64.0  /*63.817818 */
#define STARTING_FREQ_3				64.0  /*74.238716 */

#define NUM_CEP_COEFF				13  /* c1..c12 + c0 */

namespace dsp
{
	MFCC::MFCC() : native_byte_order(IEEE_LE), sampling_frequency(8000), no_c0(false), no_log_e(false), 
		swap_specified(false)
	{
		/*----------------*/
		/* Initialization */
		/*----------------*/
		energy_floor_fb = (float) exp((double) ENERGYFLOOR_FB);
		energy_floor_log_e = (float) exp((double) ENERGYFLOOR_logE);

		/*-------------------------------------------------------------------*/
		/* Set parameters FrameLength, FrameShift and FFTLength according to */
		/* the current sampling frequency                                    */
		/*-------------------------------------------------------------------*/
		if (sampling_frequency == SAMPLING_FREQ_1 * 1000) {
			frame_length = FRAME_LENGTH_1;
			frame_shift = FRAME_SHIFT_1;
			fft_length = FFT_LENGTH_1;
			starting_frequency = STARTING_FREQ_1;
		} else if (sampling_frequency == SAMPLING_FREQ_2 * 1000) {
			frame_length = FRAME_LENGTH_2;
			frame_shift = FRAME_SHIFT_2;
			fft_length = FFT_LENGTH_2;
			starting_frequency = STARTING_FREQ_2;
		} else if (sampling_frequency == SAMPLING_FREQ_3 * 1000) {
			frame_length = FRAME_LENGTH_3;
			frame_shift = FRAME_SHIFT_3;
			fft_length = FFT_LENGTH_3;
			starting_frequency = STARTING_FREQ_3;
		} else {
			printf("    ERROR:   Invalid sampling frequency '%d'!\n", sampling_frequency);
		}

		/*------------------------------------------------*/
		/* Memory allocation and initialization for input */
		/* circular float buffer                          */
		/*------------------------------------------------*/
		cfb_size = frame_length + 2;
		circular_float_buffer = new float[cfb_size];
		circular_float_buffer[0] = 0.0;
		circular_float_buffer[1] = 0.0;
		cfb_pointer = 0;

		/*-------------------------------------------------------*/
		/* Initialization of FE data structures and input buffer */
		/*-------------------------------------------------------*/
		first_window = (FFT_Window *) malloc(sizeof(FFT_Window));
		InitializeHamming(float_window, frame_length);
		InitFFTWindows(first_window, starting_frequency, (float) sampling_frequency, fft_length, NUM_CHANNELS);
		ComputeTriangle(first_window);
		dct_matrix = InitDCTMatrix(NUM_CEP_COEFF, NUM_CHANNELS);
	}

	MFCC::~MFCC()
	{
		delete[] circular_float_buffer;
		free(dct_matrix);
		ReleaseFFTWindows(first_window);
		free(first_window);
	}

	int MFCC::ReadBlock(float *block, int block_pos, int block_size, float *float_buffer, int buffer_size, 
			int buffer_pointer, int samples)
	{
		int end_pos = samples + block_pos;
		if (block_pos >= block_size) {
			return -1;
		}
		int i;
		for (i = block_pos; i < end_pos && i < block_size; ++i) {
			float_buffer[buffer_pointer] = block[i];
			buffer_pointer = (buffer_pointer + 1) % buffer_size;
		}
		for (int j = i; j < end_pos; ++j) {
			float_buffer[buffer_pointer] = 0;
			buffer_pointer = (buffer_pointer + 1) % buffer_size;
		}
		return end_pos;
	}

	void MFCC::Process(char *_block, int _block_size, std::vector<float *> &cep_coeffs)
	{
		bool must_swap = (swap_specified || native_byte_order == IEEE_LE);

		int block_size = _block_size / 2;
		float *block = new float[_block_size];
		for (int i = 0; i < block_size; ++i) {
			unsigned char a = _block[i * 2];
			unsigned char b = _block[i * 2 + 1];
			short s;
			if (must_swap) {
				s = (a) | (b << 8);
			} else {
				s = (a << 8) | (b);
			}
			block[i] = (float) s;
		}

		float log_energy;
		int frame_count = 0;
		int read_bytes = 0;

		int end_pos = ReadBlock(block, 0, block_size, circular_float_buffer, cfb_size, cfb_pointer + 2, frame_length - frame_shift);

		DCOffsetFilter(circular_float_buffer, cfb_size, &cfb_pointer, frame_length - frame_shift);

		/*----------------------------------------------------------------*/
		/*                       Framing                                  */
		/*----------------------------------------------------------------*/
		while ((end_pos = ReadBlock(block, end_pos, block_size, circular_float_buffer, cfb_size, (cfb_pointer + 2) % cfb_size, frame_shift)) != -1) {
			/*-------------------*/
			/* DC offset removal */
			/*-------------------*/
			DCOffsetFilter(circular_float_buffer, cfb_size, &cfb_pointer, frame_shift);

			/*------------------*/
			/* logE computation */
			/*------------------*/
			log_energy = 0.0;
			for (int i = 0; i < frame_length; ++i) {
				int ind = (cfb_pointer + i + 3) % cfb_size;
				log_energy += circular_float_buffer[ind] * circular_float_buffer[ind];
			}

			if (log_energy < energy_floor_log_e) {
				log_energy = ENERGYFLOOR_logE;
			} else {
				log_energy = log(log_energy);
			}
	    
			/*-----------------------------------------------------*/
			/* Pre-emphasis, moving from circular to linear buffer */
			/*-----------------------------------------------------*/
			for (int i = 0; i < frame_length; i++) {
				int a = (cfb_pointer + i + 3) % cfb_size;
				int b = (cfb_pointer + i + 2) % cfb_size;
				float_buffer[i] = circular_float_buffer[a] - ((float) PRE_EMPHASIS) * circular_float_buffer[b];
			}

			/*-----------*/
			/* Windowing */
			/*-----------*/
			Window(float_buffer, float_window, frame_length);

			/*-----*/
			/* FFT */
			/*-----*/
			/* Zero padding */
			for (int i = frame_length; i < fft_length; i++) {
				float_buffer[i] = 0.0;
			}

			/* Real valued, in-place split-radix FFT */
			int tmp_int = (int) (log10((float) fft_length) / log10(2.0f));
			rfft(float_buffer, fft_length, tmp_int); /*tmp_int = log2(FFTLength)*/

			/* Magnitude spectrum */
			float_buffer[0] = abs(float_buffer[0]);  /* DC */
			for (int i = 1; i < fft_length / 2; ++i) { /* pi/(N/2), 2pi/(N/2), ..., (N/2-1)*pi/(N/2) */
				float_buffer[i] = sqrt(float_buffer[i] * float_buffer[i] + float_buffer[fft_length - i] * float_buffer[fft_length - i]);
			}
			float_buffer[fft_length / 2] = abs(float_buffer[fft_length / 2]);  /* pi/2 */

			/*---------------*/
			/* Mel filtering */
			/*---------------*/
			MelFilterBank(float_buffer, first_window);

			/*-------------------------------*/
			/* Natural logarithm computation */
			/*-------------------------------*/
			for (int i = 0; i < NUM_CHANNELS; ++i) {
				if (float_buffer[i] < energy_floor_fb) {
					float_buffer[i] = ENERGYFLOOR_FB;
				} else {
					float_buffer[i] = log(float_buffer[i]);
				}
			}

			/*---------------------------*/
			/* Discrete Cosine Transform */
			/*---------------------------*/
			DCT(float_buffer, dct_matrix, NUM_CEP_COEFF, NUM_CHANNELS);

			/*--------------------------------------*/
			/* Append logE after c0 or overwrite c0 */
			/*--------------------------------------*/
			float_buffer[NUM_CHANNELS + NUM_CEP_COEFF - (no_c0 ? 1:0)] = log_energy; 

			/*---------------*/
			/* Output result */
			/*---------------*/
			
			int coeff_number = (int) (NUM_CEP_COEFF - (no_c0 ? 1:0) + (no_log_e ? 0:1));
			float *cep_coeff = new float[coeff_number];
			for (int i = 0; i < coeff_number; ++i) {
				cep_coeff[i] = float_buffer[NUM_CHANNELS + i];
			}
			cep_coeffs.push_back(cep_coeff);
			frame_count++;
		}

		delete[] block;
	}
}
