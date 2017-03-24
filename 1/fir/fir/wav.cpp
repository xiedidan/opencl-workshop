#include "stdafx.h"
#include <iostream>
#include <fstream>
using namespace std;

#include "wav.h"

RIFF_HEADER* riffHeader = NULL;
FMT_BLOCK* fmtBlock = NULL;
DATA_BLOCK* dataBlock = NULL;

int16_t* leftChannelData;
int16_t* rightChannelData;

int readWavHeader(const char* filename) {
	if (!riffHeader)
		riffHeader = (RIFF_HEADER*)malloc(sizeof(RIFF_HEADER));

	if (!fmtBlock)
		fmtBlock = (FMT_BLOCK*)malloc(sizeof(FMT_BLOCK));

	if (!dataBlock)
		dataBlock = (DATA_BLOCK*)malloc(sizeof(DATA_BLOCK));

	fstream fs;
	fs.open(filename, ios::binary | ios::in);

	fs.read((char*)riffHeader, sizeof(RIFF_HEADER));
	fs.read((char*)fmtBlock, sizeof(FMT_BLOCK));
	fs.read((char*)dataBlock, sizeof(DATA_BLOCK));

	fs.close();

	return 0;
}

int readWavData(const char* filename) {
	readWavHeader(filename);

	int sampleBytes = sizeof(int16_t);
	int sampleCountPerChannel = dataBlock->dataSize / 2 / sampleBytes;

	leftChannelData = (int16_t*)malloc(sampleCountPerChannel * sampleBytes);
	rightChannelData = (int16_t*)malloc(sampleCountPerChannel * sampleBytes);

	int16_t* data = (int16_t*)malloc(sampleCountPerChannel * sampleBytes * 2);

	fstream fs;
	fs.open(filename, ios::binary | ios::in);
	fs.seekg(0x2c); // skip header
	fs.read((char*)data, sampleCountPerChannel * sampleBytes * 2);
	fs.close();

	for (int i = 0; i < sampleCountPerChannel; i++) {
		leftChannelData[i] = data[i * 2];
		rightChannelData[i] = data[i * 2 + 1];
	}

	free(data);

	return 0;
}

int writeWavHeader(const char* filename, int32_t dataSize) {
	RIFF_HEADER* rh = (RIFF_HEADER*)malloc(sizeof(RIFF_HEADER));
	FMT_BLOCK* fmt = (FMT_BLOCK*)malloc(sizeof(FMT_BLOCK));
	DATA_BLOCK* d = (DATA_BLOCK*)malloc(sizeof(DATA_BLOCK));

	strncpy(rh->riffId, "RIFF", 4);
	rh->riffSize = dataSize + sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK) + sizeof(DATA_BLOCK) - 8;
	strncpy(rh->riffFormat, "WAVE", 4);

	strncpy(fmt->fmtId, "fmt ", 4);
	fmt->fmtSize = 16;
	fmt->fmtTag = 1;
	fmt->channels = 2;
	fmt->sampleRate = 176400;
	fmt->byteRate = fmt->sampleRate * fmt->channels * sizeof(int16_t);
	fmt->blockAlign = 4;
	fmt->bitsPerSample = 16;

	strncpy(d->dataId, "data", 4);
	d->dataSize = dataSize;

	fstream fs;
	fs.open(filename, ios::binary | ios::out);

	fs.write((const char*)rh, sizeof(RIFF_HEADER));
	fs.write((const char*)fmt, sizeof(FMT_BLOCK));
	fs.write((const char*)d, sizeof(DATA_BLOCK));

	fs.close();

	return 0;
}

int writeWavData(const char* filename, int16_t* leftData, int16_t* rightData, int sampleCountPerChannel) {
	int sampleBytes = sizeof(int16_t);
	int dataSize = sampleCountPerChannel * 2 * sampleBytes;

	writeWavHeader(filename, dataSize);

	int16_t* data = (int16_t*)malloc(dataSize);

	for (int i = 0; i < sampleCountPerChannel; i++) {
		data[i * 2] = leftData[i];
		data[i * 2 + 1] = rightData[i];
	}

	fstream fs;
	fs.open(filename, ios::binary | ios::out | ios::app);
	fs.write((const char*)data, dataSize);
	fs.close();

	free(data);

	return 0;
}