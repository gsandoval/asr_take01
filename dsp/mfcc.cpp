
#include "mfcc.h"

extern "C" {
#include "FEfunc.h"
}

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace dsp
{
	const float ENERGYFLOOR_FB = -50.0;		/*0.0 */
	const float ENERGYFLOOR_logE = -50.0;	/*4.0 */

	const int IEEE_LE = 0;
	const int IEEE_BE = 1;

	const int SAMPLING_FREQ_1 = 8;			/*8kHz */
	const int SAMPLING_FREQ_2 = 11;			/*11kHz */
	const int SAMPLING_FREQ_3 = 16;			/*16kHz */

	const int FRAME_LENGTH_1 = 200;			/*25ms */
	const int FRAME_LENGTH_2 = 256;			/*23.27ms */
	const int FRAME_LENGTH_3 = 400;			/*25ms */

	const int FRAME_SHIFT_1 = 80;			/*10ms */
	const int FRAME_SHIFT_2 = 110;			/*10ms */
	const int FRAME_SHIFT_3 = 160;			/*10ms */

	const float PRE_EMPHASIS = 0.97f;

	const int FFT_LENGTH_1 = 256;
	const int FFT_LENGTH_2 = 256;
	const int FFT_LENGTH_3 = 512;

	const int NUM_CHANNELS = 23;
	const float STARTING_FREQ_1 = 64.0f;		/*55.401825 */
	const float STARTING_FREQ_2 = 64.0f;		/*63.817818 */
	const float STARTING_FREQ_3 = 64.0f;		/*74.238716 */

	const int NUM_CEP_COEFF = 13;			/* c1..c12 + c0 */

	MFCC::MFCC() : native_byte_order(IEEE_LE), no_coefficient_zero(false), no_log_energy(false), swap_specified(false),
		frame_length(0), frame_shift(0), fft_length(0)
	{
		energy_floor_fb = exp(ENERGYFLOOR_FB);
		energy_floor_log_e = exp(ENERGYFLOOR_logE);
		cepstral_coefficient_number = NUM_CEP_COEFF;
	}

	void MFCC::SetCepstralCoefficientsNumber(int cepstral_coefficient_number)
	{
		this->cepstral_coefficient_number = cepstral_coefficient_number;
	}

	void MFCC::SetSamplingFrequency(int sampling_frequency)
	{
		this->sampling_frequency = sampling_frequency;
		starting_frequency = STARTING_FREQ_1; // They are all the same
		if (fft_length != 0 || frame_length != 0 || frame_shift != 0) {
			return;
		}

		if (sampling_frequency == SAMPLING_FREQ_1 * 1000) {
			frame_length = FRAME_LENGTH_1;
			frame_shift = FRAME_SHIFT_1;
			fft_length = FFT_LENGTH_1;	
		} else if (sampling_frequency == SAMPLING_FREQ_2 * 1000) {
			frame_length = FRAME_LENGTH_2;
			frame_shift = FRAME_SHIFT_2;
			fft_length = FFT_LENGTH_2;
		} else if (sampling_frequency == SAMPLING_FREQ_3 * 1000) {
			frame_length = FRAME_LENGTH_3;
			frame_shift = FRAME_SHIFT_3;
			fft_length = FFT_LENGTH_3;
		}
	}

	void MFCC::SetFrameShift(int frame_shift)
	{
		this->frame_shift = frame_shift;
	}

	void MFCC::SetFrameLength(int frame_length)
	{
		this->frame_length = frame_length;
	}

	void MFCC::SetFFTFrameLength(int fft_length)
	{
		this->fft_length = fft_length;
	}

	void MFCC::Init()
	{
		cfb_size = frame_length + 2;
		circular_float_buffer = new float[cfb_size];
		circular_float_buffer[0] = 0.0;
		circular_float_buffer[1] = 0.0;
		cfb_pointer = 0;

		float_buffer = new float[fft_length + 1];
		float_window = new float[frame_length / 2 + 1];

		first_window = (FFT_Window *) malloc(sizeof(FFT_Window));
		InitializeHamming(float_window, frame_length);
		InitFFTWindows(first_window, starting_frequency, (float) sampling_frequency, fft_length, NUM_CHANNELS);
		ComputeTriangle(first_window);
		dct_matrix = InitDCTMatrix(cepstral_coefficient_number, NUM_CHANNELS);
	}

	MFCC::~MFCC()
	{
		delete[] float_buffer;
		delete[] float_window;
		delete[] circular_float_buffer;
		free(dct_matrix);
		ReleaseFFTWindows(first_window);
		free(first_window);
	}

	void MFCC::SetNoCoefficientZero(bool no_coefficient_zero)
	{
		this->no_coefficient_zero = no_coefficient_zero;
	}

	void MFCC::SetNoLogEnergy(bool no_log_energy)
	{
		this->no_log_energy = no_log_energy;
	}

	int MFCC::ReadBlock(float *block, int block_pos, int block_size, float *float_buffer, int buffer_size, 
			int buffer_pointer, int samples)
	{
		int end_pos = samples + block_pos;
		if (block_pos >= block_size) {
			return -1;
		}
		if (end_pos >= block_size) {
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
		float *block = new float[block_size];
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
				float_buffer[i] = circular_float_buffer[a] - PRE_EMPHASIS * circular_float_buffer[b];
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
			DCT(float_buffer, dct_matrix, cepstral_coefficient_number, NUM_CHANNELS);

			/*--------------------------------------*/
			/* Append logE after c0 or overwrite c0 */
			/*--------------------------------------*/
			float_buffer[NUM_CHANNELS + cepstral_coefficient_number - (no_coefficient_zero ? 1:0)] = log_energy; 

			/*---------------*/
			/* Output result */
			/*---------------*/
			int coeff_number = (int) (cepstral_coefficient_number - (no_coefficient_zero ? 1:0) + (no_log_energy ? 0:1));
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
