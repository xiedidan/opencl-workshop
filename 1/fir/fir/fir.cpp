// fir.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
using namespace std;

#include <CL/cl.h>

#include "wav.h"
#include "os.h"
#include "os_cl.h"

const char* wavFilename = "..\\..\\..\\data\\test.wav";
const char* osFilename = "..\\..\\..\\data\\os.wav";
const char* firFilename = "..\\..\\..\\data\\lpf-176.4k.dat";
const char* clProgramPath = "..\\..\\..\\data\\fir.cl";

int main(int argc, char **argv)
{
	readWavData(wavFilename);

	cout << "Filename: \t\t" << wavFilename << endl;
	cout << "File Size:	\t" << riffHeader->riffSize + 8 << endl;
	cout << "Channels: \t\t" << fmtBlock->channels << endl;
	cout << "Sample Rate: \t\t" << fmtBlock->sampleRate << endl;
	cout << "Byte Rate: \t\t" << fmtBlock->byteRate << endl;
	cout << "Bits per Sample: \t" << fmtBlock->bitsPerSample << endl;
	cout << "Data Size: \t\t" << dataBlock->dataSize << endl;
	cout << endl;

	/*
	for (int i = 0; i < 64; i++) {
		cout << "(" << leftChannelData[i + 1024 * 1024] << ", ";
		cout << rightChannelData[i + 1024 * 1024] << ") " << endl;
	}
	cout << endl;
	*/

	readFir(firFilename, &coefficients, &tapCount);
	float** banks = createBank(coefficients, tapCount, osRatio);

	int sampleCountPerChannel = dataBlock->dataSize / 2 / sizeof(int16_t);

	/*
	int16_t* left = overSampling(leftChannelData, sampleCountPerChannel, banks, tapCount, osRatio);
	for (int i = 0; i < 64; i++) {
		cout << (int16_t)left[i + 4 * 1024 * 1024] << endl;
	}
	cout << endl;

	int16_t* right = overSampling(rightChannelData, sampleCountPerChannel, banks, tapCount, osRatio);

	writeWavData(osFilename, left, right, sampleCountPerChannel * osRatio);
	*/

	cl_platform_id platform = getPlatform();
	cout << endl;

	cl_device_id device = getDevices(platform);

	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);

	cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, NULL);

	cl_program program = getProgram(clProgramPath, context, device);

	cout << endl;
	printClBuildingLog(program, device);

	cl_kernel kernel = clCreateKernel(program, "fir", NULL);

	cl_float* clBanks = createClBank(coefficients, tapCount, osRatio);
	cl_short* extendedLeftData = prepareData(leftChannelData, sampleCountPerChannel, tapCount);
	cl_short* extendedRightData = prepareData(rightChannelData, sampleCountPerChannel, tapCount);

	int osSampleCount = sampleCountPerChannel * osRatio;
	cl_short* osLeft = (cl_short*)malloc(osSampleCount * sizeof(cl_short));
	cl_short* osRight = (cl_short*)malloc(osSampleCount * sizeof(cl_short));

	cl_int bankSize = tapCount / osRatio;

	// left channel
	cl_mem banksBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * tapCount, clBanks, NULL);
	cl_mem leftBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_short) * sampleCountPerChannel, extendedLeftData, NULL);
	cl_mem osLeftBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_short) * osSampleCount, NULL, NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &banksBuffer);
	clSetKernelArg(kernel, 1, sizeof(cl_int), &bankSize);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &leftBuffer);
	clSetKernelArg(kernel, 3, sizeof(cl_int), &sampleCountPerChannel);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), &osLeftBuffer);
	clSetKernelArg(kernel, 5, sizeof(cl_int), &osRatio);

	size_t globalWorkSize[1] = { sampleCountPerChannel };
	cl_int status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	cout << "clEnqueueNDRangeKernel " << status << endl;

	status = clEnqueueReadBuffer(commandQueue, osLeftBuffer, CL_TRUE, 0, sizeof(cl_short) * osSampleCount, osLeft, 0, NULL, NULL);
	cout << "clEnqueueReadBuffer " << status << endl;

	// right channel
	cl_mem rightBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_short) * sampleCountPerChannel, extendedRightData, NULL);
	cl_mem osRightBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_short) * osSampleCount, NULL, NULL);

	clSetKernelArg(kernel, 2, sizeof(cl_mem), &rightBuffer);
	clSetKernelArg(kernel, 4, sizeof(cl_mem), &osRightBuffer);

	status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	cout << "clEnqueueNDRangeKernel " << status << endl;

	status = clEnqueueReadBuffer(commandQueue, osRightBuffer, CL_TRUE, 0, sizeof(cl_short) * osSampleCount, osRight, 0, NULL, NULL);
	cout << "clEnqueueReadBuffer " << status << endl;

	cout << endl;

	writeWavData(osFilename, osLeft, osRight, sampleCountPerChannel * osRatio);
	cout << "Output Filename: \t" << osFilename << endl;

	cout << endl;

	free(coefficients);
	for (int i = 0; i < osRatio; i++)
		free(banks[i]);
	free(banks);
	free(clBanks);
	free(leftChannelData);
	free(rightChannelData);
	free(osLeft);
	free(osRight);
	free(extendedLeftData);
	free(extendedRightData);

	status = clReleaseKernel(kernel);
	status = clReleaseProgram(program);
	status = clReleaseMemObject(banksBuffer);
	status = clReleaseMemObject(leftBuffer);
	status = clReleaseMemObject(osLeftBuffer);
	status = clReleaseMemObject(rightBuffer);
	status = clReleaseMemObject(osRightBuffer);
	status = clReleaseCommandQueue(commandQueue);
	status = clReleaseContext(context);

	system("pause");

	return 0;
}

