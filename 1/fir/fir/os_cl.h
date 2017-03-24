#pragma once

// int16_t* overSampling(int16_t* data, int sampleCount, float** banks, int tapCount, int osRatio);

// api
cl_float* createClBank(float* coefficients, int tapCount, int osRatio);
cl_short* prepareData(int16_t* data, int sampleCount, int tapCount);

// opencl helpers
cl_platform_id getPlatform();
cl_device_id getDevices(cl_platform_id platform);
cl_program getProgram(const char* programPath, cl_context context, cl_device_id device);
void printClBuildingLog(cl_program program, cl_device_id device);
