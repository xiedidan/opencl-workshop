__kernel void fir(__global float* banks, int bankSize,
  __global const short* samples, int sampleCount,
  __global short* results, int osRatio) {

  int i, j, k;
  int bankOffset = 0;
  float sum = 0;

  i = get_global_id(0);

  for (j = 0; j < osRatio; j++) {
    bankOffset = j * bankSize;
    sum = 0;

    for (k = 0; k < bankSize; k++) {
      sum += banks[bankOffset + k] * samples[i + k];
    }

    results[i * osRatio + j] = (short)sum;
  }
}
