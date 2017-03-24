#include "stdafx.h"
#include <iostream>
#include <fstream>
using namespace std;

#include "os.h"

float* coefficients;
int tapCount;

int16_t* overSampling(int16_t* data, int sampleCount, float** banks, int tapCount, int osRatio) {
	// add tapCount-1 0s before data
	int tempSize = sampleCount + tapCount - 1;
	int16_t* tempData = (int16_t*)malloc(tempSize * sizeof(int16_t));
	memset(tempData, 0, tempSize * sizeof(int16_t));
	memcpy(&tempData[4], data, sampleCount * sizeof(int16_t));

	int osSampleCount = sampleCount * osRatio;
	int16_t* osData = (int16_t*)malloc(osSampleCount * sizeof(int16_t));
	memset(osData, 0, osSampleCount * sizeof(int16_t));

	int bankSize = tapCount / osRatio;
	int osIndex = 0;
	for (int i = 0; i < sampleCount; i++) {
		for (int j = 0; j < osRatio; j++) {
			float* bank = banks[j];
			float sum = 0;

			for (int k = 0; k < bankSize; k++) {
				sum += bank[k] * tempData[i + k];
			}

			osData[osIndex] = sum;
			osIndex++;
		}
	}

	free(tempData);

	return osData;
}

int readFir(const char* filename, float** coefficients, int* tapCount) {
	fstream fs;
	fs.open(filename, ios::binary | ios::in);

	fs.seekg(0, ios::end);
	int fileSize = fs.tellg();
	
	*coefficients = (float*)malloc(fileSize);
	int coefSize = sizeof(float);
	*tapCount = fileSize / coefSize;

	fs.seekg(0);
	for (int i = 0; i < *tapCount; i++) {
		fs.read((char*)&((*coefficients)[i]), coefSize);
	}

	fs.close();

	return 0;
}

float** createBank(float* coefficients, int tapCount, int osRatio) {
	float** banks;
	banks = (float**)malloc(osRatio * sizeof(float*));

	int bankSize = tapCount / osRatio;

	for (int i = 0; i < osRatio; i++) {
		banks[i] = (float*)malloc(bankSize * sizeof(float));
		float* bank = banks[i];

		for (int j = 0; j < bankSize; j++) {
			bank[j] = coefficients[j * osRatio + i];
		}
	}

	return banks;
}