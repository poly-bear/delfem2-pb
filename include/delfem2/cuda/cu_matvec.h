#include <cstdio>
#include <cstdlib>
#include "cuda_runtime.h"

__global__
void kernel_VecScale(float *out, float *in, float scale, const int n);

void cuda_VecScale(float *hOut, float *hIn, float scale, const int n);

// -------------------------------------------------------------

/**
 * @brief dot product of two vectors
 */
__global__
void cuda_Dot_TPB64(
    float* d_res,
    const float* d_A,
    const float* d_B,
    int n);

float cuda_Dot(
    const float* h_A,
    const float* h_B,
    unsigned int n);

// -------------------------------------------------------------

__global__
void kernel_MatMat_TPB16(
    float *C,
    const float *A,
    const float *B,
    unsigned int WIDTH);

void cuda_MatMat(
    float* h_C_gpu,
    const float* h_A,
    const float* h_B,
    unsigned int WIDTH);