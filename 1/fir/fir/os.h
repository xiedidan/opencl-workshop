#pragma once

const int osRatio = 4;

extern float* coefficients;
extern int tapCount;

int16_t* overSampling(int16_t* data, int sampleCount, float** banks, int tapCount, int osRatio);
int readFir(const char* filename, float** coefficients, int* tapCount);
float** createBank(float* coefficients, int tapCount, int osRatio);
