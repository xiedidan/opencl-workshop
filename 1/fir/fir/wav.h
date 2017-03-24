#pragma once

// this is not a generic preposed wav reader
// it's used for stereo 16bit files, and suppose there's no fact block in wav header

typedef struct _RIFF_HEADER {
	char riffId[4];
	int32_t riffSize;
	char riffFormat[4];
} RIFF_HEADER;

typedef struct _FMT_BLOCK {
	char fmtId[4];
	int32_t fmtSize;
	int16_t fmtTag;
	int16_t channels;
	int32_t sampleRate;
	int32_t byteRate;
	int16_t blockAlign;
	int16_t bitsPerSample;
} FMT_BLOCK;

typedef struct _DATA_BLOCK {
	char dataId[4];
	int32_t dataSize;
} DATA_BLOCK;

extern RIFF_HEADER* riffHeader;
extern FMT_BLOCK* fmtBlock;
extern DATA_BLOCK* dataBlock;

extern int16_t* leftChannelData;
extern int16_t* rightChannelData;

int readWavData(const char* filename);
int writeWavData(const char* filename, int16_t* leftData, int16_t* rightData, int sampleCountPerChannel);
