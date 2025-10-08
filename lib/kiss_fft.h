/*
 *  Copyright (c) 2003-2010, Mark Borgerding. All rights reserved.
 *  This file is part of KISS FFT - https://github.com/mborgerding/kissfft
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  See COPYING file for more information.
 */

// Compile the following section when KISS_FFT_H is not defined.
#ifndef KISS_FFT_H
// Define macro KISS_FFT_H.
#define KISS_FFT_H

// Bring in declarations from <stdlib.h>.
#include <stdlib.h>
// Bring in declarations from <stdio.h>.
#include <stdio.h>
// Bring in declarations from <math.h>.
#include <math.h>
// Bring in declarations from <string.h>.
#include <string.h>

// Only compile the following section when __cplusplus is defined.
#ifdef __cplusplus
// Execute extern "C" {.
extern "C" {
// Close the preceding conditional compilation block.
#endif

/*
 ATTENTION!
 If you would like a :
 -- a utility that will handle the caching of fft objects
 -- real-only (no imaginary time component ) FFT
 -- a multi-dimensional FFT
 -- a command-line utility to perform ffts
 -- a command-line utility to perform fast-convolution filtering

 Then see kfc.h kiss_fftr.h kiss_fftnd.h fftutil.c kiss_fastfir.c
  in the tools/ directory.
*/
/* User may override KISS_FFT_MALLOC and/or KISS_FFT_FREE. */
// Only compile the following section when USE_SIMD is defined.
#ifdef USE_SIMD
// Execute # include <xmmintrin.h>.
# include <xmmintrin.h>
// Execute # define kiss_fft_scalar __m128.
# define kiss_fft_scalar __m128
// Execute # ifndef KISS_FFT_MALLOC.
# ifndef KISS_FFT_MALLOC
// Evaluate expression #  define KISS_FFT_MALLOC(nbytes) _mm_malloc(nbytes,16).
#  define KISS_FFT_MALLOC(nbytes) _mm_malloc(nbytes,16)
// Execute # endif.
# endif
// Execute # ifndef KISS_FFT_FREE.
# ifndef KISS_FFT_FREE
// Execute #  define KISS_FFT_FREE _mm_free.
#  define KISS_FFT_FREE _mm_free
// Execute # endif.
# endif
// Execute #else.
#else
// Execute # ifndef KISS_FFT_MALLOC.
# ifndef KISS_FFT_MALLOC
// Execute #  define KISS_FFT_MALLOC malloc.
#  define KISS_FFT_MALLOC malloc
// Execute # endif.
# endif
// Execute # ifndef KISS_FFT_FREE.
# ifndef KISS_FFT_FREE
// Execute #  define KISS_FFT_FREE free.
#  define KISS_FFT_FREE free
// Execute # endif.
# endif
// Close the preceding conditional compilation block.
#endif


// Only compile the following section when FIXED_POINT is defined.
#ifdef FIXED_POINT
// Bring in declarations from <stdint.h>.
#include <stdint.h>
// Evaluate expression # if (FIXED_POINT == 32).
# if (FIXED_POINT == 32)
// Execute #  define kiss_fft_scalar int32_t.
#  define kiss_fft_scalar int32_t
// Execute # else.
# else
// Execute #  define kiss_fft_scalar int16_t.
#  define kiss_fft_scalar int16_t
// Execute # endif.
# endif
// Execute #else.
#else
// Execute # ifndef kiss_fft_scalar.
# ifndef kiss_fft_scalar
/*  default is float */
// Execute #   define kiss_fft_scalar float.
#   define kiss_fft_scalar float
// Execute # endif.
# endif
// Close the preceding conditional compilation block.
#endif

// Create typedef struct {.
typedef struct {
    // Declare kiss_fft_scalar r.
    kiss_fft_scalar r;
    // Declare kiss_fft_scalar i.
    kiss_fft_scalar i;
// Close the current scope and emit the trailing comment.
}kiss_fft_cpx;

// Create typedef struct kiss_fft_state* kiss_fft_cfg.
typedef struct kiss_fft_state* kiss_fft_cfg;

/*
 *  kiss_fft_alloc
 *
 *  Initialize a FFT (or IFFT) algorithm's cfg/state buffer.
 *
 *  typical usage:      kiss_fft_cfg mycfg=kiss_fft_alloc(1024,0,NULL,NULL);
 *
 *  The return value from fft_alloc is a cfg buffer used internally
 *  by the fft routine or NULL.
 *
 *  If lenmem is NULL, then kiss_fft_alloc will allocate a cfg buffer using malloc.
 *  The returned value should be free()d when done to avoid memory leaks.
 *
 *  The state can be placed in a user supplied buffer 'mem':
 *  If lenmem is not NULL and mem is not NULL and *lenmem is large enough,
 *      then the function places the cfg in mem and the size used in *lenmem
 *      and returns mem.
 *
 *  If lenmem is not NULL and ( mem is NULL or *lenmem is not large enough),
 *      then the function returns NULL and places the minimum cfg
 *      buffer size in *lenmem.
 * */

// Execute statement kiss_fft_cfg kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem).
kiss_fft_cfg kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem);

/*
 * kiss_fft(cfg,in_out_buf)
 *
 * Perform an FFT on a complex input buffer.
 * for a forward FFT,
 * fin should be  f[0] , f[1] , ... ,f[nfft-1]
 * fout will be   F[0] , F[1] , ... ,F[nfft-1]
 * Note that each element is complex and can be accessed like
    f[k].r and f[k].i
 * */
// Execute statement void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout).
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);

/*
 A more generic version of the above function. It reads its input from every Nth sample.
 * */
// Execute statement void kiss_fft_stride(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout,int fin_stride).
void kiss_fft_stride(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout,int fin_stride);

/* If kiss_fft_alloc allocated a buffer, it is one contiguous
   buffer and can be simply free()d when no longer needed*/
// Define macro kiss_fft_free expanding to KISS_FFT_FREE.
#define kiss_fft_free KISS_FFT_FREE

/*
 Cleans up some memory that gets managed internally. Not necessary to call, but it might clean up
 your compiler output to call this before you exit.
*/
// Execute statement void kiss_fft_cleanup(void).
void kiss_fft_cleanup(void);


/*
 * Returns the smallest integer k, such that k>=n and k has only "fast" factors (2,3,5)
 */
// Execute statement int kiss_fft_next_fast_size(int n).
int kiss_fft_next_fast_size(int n);

/* for real ffts, we need an even size */
// Define macro kiss_fftr_next_fast_size_real(n) expanding to \.
#define kiss_fftr_next_fast_size_real(n) \
        // Evaluate expression (kiss_fft_next_fast_size( ((n)+1)>>1)<<1).
        (kiss_fft_next_fast_size( ((n)+1)>>1)<<1)


// Only compile the following section when __cplusplus is defined.
#ifdef __cplusplus
// Close the current scope block.
}
// Close the preceding conditional compilation block.
#endif

// Close the preceding conditional compilation block.
#endif
