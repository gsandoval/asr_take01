#include "audio_controller.h"
#include "audio_device.h"
#include "raw_audio_listener.h"

#include <vector>
#include <iostream>
#include <string>

using namespace std;

class RawAudioListenerImpl : public audiocapture::RawAudioListener
{
private:
	bool opened_file;
	HMMIO file_handle;
	MMCKINFO chunk_info;
	MMCKINFO data_chunk_info;
public:
	RawAudioListenerImpl() : opened_file(false)
	{
	}

	void CreateOutputFile(string filename)
	{
		wchar_t *file = new wchar_t[filename.size() + 1];
		memset(file, 0, filename.size() + 1);
		MultiByteToWideChar(CP_ACP, NULL, filename.c_str(), -1, file, filename.size() + 1);
		file_handle = mmioOpenW(file, 0, MMIO_CREATE | MMIO_WRITE);
		delete[] file;

		chunk_info.fccType = mmioStringToFOURCCA("WAVE", 0);
		MMRESULT mr = mmioCreateChunk(file_handle, &chunk_info, MMIO_CREATERIFF);

		MMCKINFO wave_fmt;
		wave_fmt.ckid = mmioStringToFOURCCA("fmt ", 0);
		wave_fmt.cksize = sizeof(WAVEFORMATEX);
		mr = mmioCreateChunk(file_handle, &wave_fmt, 0);
		WAVEFORMATEX header;
		header.wFormatTag = WAVE_FORMAT_PCM;
		header.nChannels = 1;
		header.wBitsPerSample = 8;
		header.nSamplesPerSec = 8000;
		header.nBlockAlign = header.nChannels * (header.wBitsPerSample / 8);
		header.nAvgBytesPerSec = header.nSamplesPerSec * header.nBlockAlign;
		header.cbSize = 0;
		long bytes = mmioWrite(file_handle, (char*) &header, sizeof(WAVEFORMATEX) - 2);
		mr = mmioAscend(file_handle, &wave_fmt, 0);

		memset(&data_chunk_info, 0, sizeof(MMCKINFO));
		data_chunk_info.ckid = mmioStringToFOURCCA("data", 0);
		//data_chunk_info.cksize = block_size;
		mr = mmioCreateChunk(file_handle, &data_chunk_info, 0);

		opened_file = true;
	}

	void CloseOutputFile()
	{
		opened_file = false;
		MMRESULT mr = mmioAscend(file_handle, &data_chunk_info, 0);
		mr = mmioAscend(file_handle, &chunk_info, 0);
		mr = mmioClose(file_handle, 0);
	}

	void BlockRead(char *block, int block_size)
	{
		if (opened_file) {
			long bytes = mmioWrite(file_handle, block, block_size);
		}
	}
};

audiocapture::AudioController* CreateAudioController()
{
	int channels = 1;
	int bits_per_sample = 8;
	int samples_per_second = 8000;
	int buffer_size = (samples_per_second * channels * (bits_per_sample / 8)) << 1;
	audiocapture::AudioController *ac = new audiocapture::AudioController(channels, samples_per_second, bits_per_sample, 3, buffer_size);
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
		cout << "Recording and Feature Extraction Utility v1.0" << endl;
		cout << "Usage:" << endl;
		cout << "\t" << argv[0] << " -r filename.wav\tFor recording audio and storing it in filename.wav" << endl;
		cout << "\t" << argv[0] << " -e filename.wav\tFor extracting filename.wav features" << endl;
		return 0;
	}
	if (strcmp(argv[1], "-r")) {
		char filename[120];
		strcpy(filename, "test01.wav");
		if (argc > 2) {
			strcpy(filename, argv[2]);
		}
		audiocapture::AudioController *ac = CreateAudioController();
		RawAudioListenerImpl listener;
		ac->AddRawAudioListener(&listener);
	
		//PrintDevices(ac);
		int selection;
		//cin >> selection;
		selection = 0;
		ac->SelectDevice(ac->ListDevices()[selection]);
	
		listener.CreateOutputFile(filename);
		ac->Start();
		cout << "GO" << endl;
		cin >> selection;
		listener.CloseOutputFile();
		ac->Stop();
	} else if (strcmp(argv[1], "-e")) {
		
	}
	return 0;
}
