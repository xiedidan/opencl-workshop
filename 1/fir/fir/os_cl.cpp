#include "stdafx.h"
#include <iomanip>
#include <iostream>
#include <fstream>
using namespace std;

#include <CL/cl.h>
#include "os_cl.h"

#define SUCCESS 0
#define FAILURE 1

/* convert the kernel file into a string */
int convertToString(const char *filename, std::string& s) {
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open()) {
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str) {
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout << "Error: failed to open file\n:" << filename << endl;
	return FAILURE;
}

cl_platform_id getPlatform() {
	cl_uint numPlatforms;
	cl_platform_id platform = NULL;

	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return NULL;
	}

	if (numPlatforms > 0) {
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));

		char platformVendor[128];
		char platformName[128];
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		cout << "Found the following platform(s): " << endl;

		for (int i = 0; i < numPlatforms; i++) {
			status = clGetPlatformInfo(platforms[i],
				CL_PLATFORM_NAME,
				sizeof(platformName),
				platformName,
				NULL);

			status = clGetPlatformInfo(platforms[i],
				CL_PLATFORM_VENDOR,
				sizeof(platformVendor),
				platformVendor,
				NULL);

			cout << i + 1 << " - [" << platformVendor << "] " << platformName << endl;
		}
		cout << "Select platform no: ";

		int platformNo = 0;
		cin >> platformNo;

		platform = platforms[platformNo - 1];
		free(platforms);

		return platform;
	}

	return NULL;
}

cl_device_id getDevices(cl_platform_id platform) {
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	cl_device_id device;

	cl_int status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
	if (numDevices == 0)
	{
		cout << "No device available." << endl;
		return NULL;
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

		char deviceName[128];
		char deviceVersion[256];
		char deviceDriverVersion[128];
		cl_device_type deviceType;
		cl_ulong deviceMem;

		cout << "Found the following device(s):" << endl;
		for (int i = 0; i < numDevices; i++) {
			status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
			status = clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(deviceVersion), deviceVersion, NULL);
			status = clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(deviceDriverVersion), deviceDriverVersion, NULL);
			status = clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);
			status = clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(deviceMem), &deviceMem, NULL);

			cout << i + 1 << " - ";
			switch (deviceType) {
			case CL_DEVICE_TYPE_CPU:
				cout << "[CPU] ";
				break;

			case CL_DEVICE_TYPE_GPU:
				cout << "[GPU] ";
				break;

			default:
				cout << "[Other] ";
				break;
			}
			cout << deviceName << ", " << setprecision(2) << ((float)deviceMem / 1024 / 1024 /1024) << "GB, " <<deviceVersion << ", " << deviceDriverVersion << endl;
		}

		cout << "Select device no: ";

		int deviceNo = 0;
		cin >> deviceNo;

		device = devices[deviceNo - 1];
		free(devices);

		return device;
	}

	return NULL;
}

cl_program getProgram(const char* programPath, cl_context context, cl_device_id device) {
	string sourceStr;
	cl_int status = convertToString(programPath, sourceStr);

	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };

	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);

	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

	return program;
}

void printClBuildingLog(cl_program program, cl_device_id device) {
	// Shows the log
	char* build_log;
	size_t log_size;

	// First call to know the proper size
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = new char[log_size + 1];

	// Second call to get the log
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';

	cout << build_log << endl;
	delete[] build_log;
}

cl_float* createClBank(float* coefficients, int tapCount, int osRatio) {
	cl_float* banks = (cl_float*)malloc(tapCount * sizeof(cl_float));
	int bankIndex = 0;

	int bankSize = tapCount / osRatio;

	for (int i = 0; i < osRatio; i++) {
		for (int j = 0; j < bankSize; j++) {
			banks[bankIndex] = coefficients[j * osRatio + i];
			bankIndex++;
		}
	}

	return banks;
}

cl_short* prepareData(int16_t* data, int sampleCount, int tapCount) {
	// add tapCount-1 0s before data
	int tempSize = sampleCount + tapCount - 1;

	cl_short* tempData = (cl_short*)malloc(tempSize * sizeof(cl_short));
	memset(tempData, 0, tempSize * sizeof(cl_short));
	memcpy(&tempData[4], data, sampleCount * sizeof(cl_short));

	return tempData;
}
