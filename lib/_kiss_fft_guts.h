/*
 *  Copyright (c) 2003-2010, Mark Borgerding. All rights reserved.
 *  This file is part of KISS FFT - https://github.com/mborgerding/kissfft
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  See COPYING file for more information.
 */

/* kiss_fft.h
   defines kiss_fft_scalar as either short or a float type
   and defines
   typedef struct { kiss_fft_scalar r; kiss_fft_scalar i; }kiss_fft_cpx; */
// Bring in declarations from "kiss_fft.h".
#include "kiss_fft.h"
// Bring in declarations from <limits.h>.
#include <limits.h>

// Define macro MAXFACTORS expanding to 32.
#define MAXFACTORS 32
/* e.g. an fft of length 128 has 4 factors 
 as far as kissfft is concerned
 4*4*4*2
 */

// Declare struct kiss_fft_state{.
struct kiss_fft_state{
    // Declare int nfft.
    int nfft;
    // Declare int inverse.
    int inverse;
    // Declare int factors[2*MAXFACTORS].
    int factors[2*MAXFACTORS];
    // Declare kiss_fft_cpx twiddles[1].
    kiss_fft_cpx twiddles[1];
// Close the current scope and emit the trailing comment.
};

/*
  Explanation of macros dealing with complex math:

   C_MUL(m,a,b)         : m = a*b
   C_FIXDIV( c , div )  : if a fixed point impl., c /= div. noop otherwise
   C_SUB( res, a,b)     : res = a - b
   C_SUBFROM( res , a)  : res -= a
   C_ADDTO( res , a)    : res += a
 * */
// Only compile the following section when FIXED_POINT is defined.
#ifdef FIXED_POINT
// Bring in declarations from <stdint.h>.
#include <stdint.h>
// Evaluate expression #if (FIXED_POINT==32).
#if (FIXED_POINT==32)
// Execute # define FRACBITS 31.
# define FRACBITS 31
// Execute # define SAMPPROD int64_t.
# define SAMPPROD int64_t
// Define macro SAMP_MAX expanding to INT32_MAX.
#define SAMP_MAX INT32_MAX
// Define macro SAMP_MIN expanding to INT32_MIN.
#define SAMP_MIN INT32_MIN
// Execute #else.
#else
// Execute # define FRACBITS 15.
# define FRACBITS 15
// Execute # define SAMPPROD int32_t.
# define SAMPPROD int32_t
// Define macro SAMP_MAX expanding to INT16_MAX.
#define SAMP_MAX INT16_MAX
// Define macro SAMP_MIN expanding to INT16_MIN.
#define SAMP_MIN INT16_MIN
// Close the preceding conditional compilation block.
#endif

// Evaluate expression #if defined(CHECK_OVERFLOW).
#if defined(CHECK_OVERFLOW)
// Execute #  define CHECK_OVERFLOW_OP(a,op,b)  \.
#  define CHECK_OVERFLOW_OP(a,op,b)  \
// Branch when condition ( (SAMPPROD)(a) op (SAMPPROD)(b) > SAMP_MAX || (SAMPPROD)(a) op (SAMPPROD)(b) < SAMP_MIN ) evaluates to true.
	if ( (SAMPPROD)(a) op (SAMPPROD)(b) > SAMP_MAX || (SAMPPROD)(a) op (SAMPPROD)(b) < SAMP_MIN ) { \
// Execute fprintf(stderr,"WARNING:overflow @ " __FILE__ "(%d): (%d " #op" %d) = %ld\n",__LINE__,(a),(b),(SAMPPROD)(a) op (SAMPPROD)(b) );  }.
		fprintf(stderr,"WARNING:overflow @ " __FILE__ "(%d): (%d " #op" %d) = %ld\n",__LINE__,(a),(b),(SAMPPROD)(a) op (SAMPPROD)(b) );  }
// Close the preceding conditional compilation block.
#endif


// Evaluate expression #   define smul(a,b) ( (SAMPPROD)(a)*(b) ).
#   define smul(a,b) ( (SAMPPROD)(a)*(b) )
// Evaluate expression #   define sround( x )  (kiss_fft_scalar)( ( (x) + (1<<(FRACBITS-1)) ) >> FRACBITS ).
#   define sround( x )  (kiss_fft_scalar)( ( (x) + (1<<(FRACBITS-1)) ) >> FRACBITS )

// Evaluate expression #   define S_MUL(a,b) sround( smul(a,b) ).
#   define S_MUL(a,b) sround( smul(a,b) )

// Execute #   define C_MUL(m,a,b) \.
#   define C_MUL(m,a,b) \
      // Start the body of a do-while loop.
      do{ (m).r = sround( smul((a).r,(b).r) - smul((a).i,(b).i) ); \
          // Evaluate expression (m).i = sround( smul((a).r,(b).i) + smul((a).i,(b).r) ); }while(0).
          (m).i = sround( smul((a).r,(b).i) + smul((a).i,(b).r) ); }while(0)

// Execute #   define DIVSCALAR(x,k) \.
#   define DIVSCALAR(x,k) \
// Evaluate expression (x) = sround( smul(  x, SAMP_MAX/k ) ).
	(x) = sround( smul(  x, SAMP_MAX/k ) )

// Execute #   define C_FIXDIV(c,div) \.
#   define C_FIXDIV(c,div) \
// Start the body of a do-while loop.
	do {    DIVSCALAR( (c).r , div);  \
// Define the R function.
		DIVSCALAR( (c).i  , div); }while (0)

// Execute #   define C_MULBYSCALAR( c, s ) \.
#   define C_MULBYSCALAR( c, s ) \
    // Start the body of a do-while loop.
    do{ (c).r =  sround( smul( (c).r , s ) ) ;\
        // Evaluate expression (c).i =  sround( smul( (c).i , s ) ) ; }while(0).
        (c).i =  sround( smul( (c).i , s ) ) ; }while(0)

// Execute #else  /* not FIXED_POINT*/.
#else  /* not FIXED_POINT*/

// Evaluate expression #   define S_MUL(a,b) ( (a)*(b) ).
#   define S_MUL(a,b) ( (a)*(b) )
// Define macro C_MUL(m,a,b) expanding to \.
#define C_MUL(m,a,b) \
    // Start the body of a do-while loop.
    do{ (m).r = (a).r*(b).r - (a).i*(b).i;\
        // Evaluate expression (m).i = (a).r*(b).i + (a).i*(b).r; }while(0).
        (m).i = (a).r*(b).i + (a).i*(b).r; }while(0)
// Execute #   define C_FIXDIV(c,div) /* NOOP */.
#   define C_FIXDIV(c,div) /* NOOP */
// Execute #   define C_MULBYSCALAR( c, s ) \.
#   define C_MULBYSCALAR( c, s ) \
    // Start the body of a do-while loop.
    do{ (c).r *= (s);\
        // Evaluate expression (c).i *= (s); }while(0).
        (c).i *= (s); }while(0)
// Close the preceding conditional compilation block.
#endif

// Compile the following section when CHECK_OVERFLOW_OP is not defined.
#ifndef CHECK_OVERFLOW_OP
// Execute #  define CHECK_OVERFLOW_OP(a,op,b) /* noop */.
#  define CHECK_OVERFLOW_OP(a,op,b) /* noop */
// Close the preceding conditional compilation block.
#endif

// Define macro C_ADD( expanding to res, a,b)\.
#define  C_ADD( res, a,b)\
    // Start the body of a do-while loop.
    do { \
// Execute CHECK_OVERFLOW_OP((a).r,+,(b).r)\.
	    CHECK_OVERFLOW_OP((a).r,+,(b).r)\
// Execute CHECK_OVERFLOW_OP((a).i,+,(b).i)\.
	    CHECK_OVERFLOW_OP((a).i,+,(b).i)\
// Execute (res).r=(a).r+(b).r;  (res).i=(a).i+(b).i; \.
	    (res).r=(a).r+(b).r;  (res).i=(a).i+(b).i; \
    // Close the current scope and emit the trailing comment.
    }while(0)
// Define macro C_SUB( expanding to res, a,b)\.
#define  C_SUB( res, a,b)\
    // Start the body of a do-while loop.
    do { \
// Execute CHECK_OVERFLOW_OP((a).r,-,(b).r)\.
	    CHECK_OVERFLOW_OP((a).r,-,(b).r)\
// Execute CHECK_OVERFLOW_OP((a).i,-,(b).i)\.
	    CHECK_OVERFLOW_OP((a).i,-,(b).i)\
// Execute (res).r=(a).r-(b).r;  (res).i=(a).i-(b).i; \.
	    (res).r=(a).r-(b).r;  (res).i=(a).i-(b).i; \
    // Close the current scope and emit the trailing comment.
    }while(0)
// Define macro C_ADDTO( expanding to res , a)\.
#define C_ADDTO( res , a)\
    // Start the body of a do-while loop.
    do { \
// Execute CHECK_OVERFLOW_OP((res).r,+,(a).r)\.
	    CHECK_OVERFLOW_OP((res).r,+,(a).r)\
// Execute CHECK_OVERFLOW_OP((res).i,+,(a).i)\.
	    CHECK_OVERFLOW_OP((res).i,+,(a).i)\
// Execute (res).r += (a).r;  (res).i += (a).i;\.
	    (res).r += (a).r;  (res).i += (a).i;\
    // Close the current scope and emit the trailing comment.
    }while(0)

// Define macro C_SUBFROM( expanding to res , a)\.
#define C_SUBFROM( res , a)\
    // Start the body of a do-while loop.
    do {\
// Execute CHECK_OVERFLOW_OP((res).r,-,(a).r)\.
	    CHECK_OVERFLOW_OP((res).r,-,(a).r)\
// Execute CHECK_OVERFLOW_OP((res).i,-,(a).i)\.
	    CHECK_OVERFLOW_OP((res).i,-,(a).i)\
// Execute (res).r -= (a).r;  (res).i -= (a).i; \.
	    (res).r -= (a).r;  (res).i -= (a).i; \
    // Close the current scope and emit the trailing comment.
    }while(0)


// Only compile the following section when FIXED_POINT is defined.
#ifdef FIXED_POINT
// Evaluate expression #  define KISS_FFT_COS(phase)  floor(.5+SAMP_MAX * cos (phase)).
#  define KISS_FFT_COS(phase)  floor(.5+SAMP_MAX * cos (phase))
// Evaluate expression #  define KISS_FFT_SIN(phase)  floor(.5+SAMP_MAX * sin (phase)).
#  define KISS_FFT_SIN(phase)  floor(.5+SAMP_MAX * sin (phase))
// Evaluate expression #  define HALF_OF(x) ((x)>>1).
#  define HALF_OF(x) ((x)>>1)
// Evaluate expression #elif defined(USE_SIMD).
#elif defined(USE_SIMD)
// Evaluate expression #  define KISS_FFT_COS(phase) _mm_set1_ps( cos(phase) ).
#  define KISS_FFT_COS(phase) _mm_set1_ps( cos(phase) )
// Evaluate expression #  define KISS_FFT_SIN(phase) _mm_set1_ps( sin(phase) ).
#  define KISS_FFT_SIN(phase) _mm_set1_ps( sin(phase) )
// Evaluate expression #  define HALF_OF(x) ((x)*_mm_set1_ps(.5)).
#  define HALF_OF(x) ((x)*_mm_set1_ps(.5))
// Execute #else.
#else
// Evaluate expression #  define KISS_FFT_COS(phase) (kiss_fft_scalar) cos(phase).
#  define KISS_FFT_COS(phase) (kiss_fft_scalar) cos(phase)
// Evaluate expression #  define KISS_FFT_SIN(phase) (kiss_fft_scalar) sin(phase).
#  define KISS_FFT_SIN(phase) (kiss_fft_scalar) sin(phase)
// Evaluate expression #  define HALF_OF(x) ((x)*.5).
#  define HALF_OF(x) ((x)*.5)
// Close the preceding conditional compilation block.
#endif

// Define macro kf_cexp(x,phase) expanding to \.
#define  kf_cexp(x,phase) \
// Start the body of a do-while loop.
	do{ \
// Execute (x)->r = KISS_FFT_COS(phase);\.
		(x)->r = KISS_FFT_COS(phase);\
// Execute (x)->i = KISS_FFT_SIN(phase);\.
		(x)->i = KISS_FFT_SIN(phase);\
// Close the current scope and emit the trailing comment.
	}while(0)


/* a debugging function */
// Define macro pcpx(c)\.
#define pcpx(c)\
    // Define the f function.
    fprintf(stderr,"%g + %gi\n",(double)((c)->r),(double)((c)->i) )


// Only compile the following section when KISS_FFT_USE_ALLOCA is defined.
#ifdef KISS_FFT_USE_ALLOCA
// define this to allow use of alloca instead of malloc for temporary buffers
// Temporary buffers are used in two case: 
// 1. FFT sizes that have "bad" factors. i.e. not 2,3 and 5
// 2. "in-place" FFTs.  Notice the quotes, since kissfft does not really do an in-place transform.
// Bring in declarations from <alloca.h>.
#include <alloca.h>
// Define macro KISS_FFT_TMP_ALLOC(nbytes) expanding to alloca(nbytes).
#define  KISS_FFT_TMP_ALLOC(nbytes) alloca(nbytes)
// Define macro KISS_FFT_TMP_FREE(ptr).
#define  KISS_FFT_TMP_FREE(ptr) 
// Execute #else.
#else
// Define macro KISS_FFT_TMP_ALLOC(nbytes) expanding to KISS_FFT_MALLOC(nbytes).
#define  KISS_FFT_TMP_ALLOC(nbytes) KISS_FFT_MALLOC(nbytes)
// Define macro KISS_FFT_TMP_FREE(ptr) expanding to KISS_FFT_FREE(ptr).
#define  KISS_FFT_TMP_FREE(ptr) KISS_FFT_FREE(ptr)
// Close the preceding conditional compilation block.
#endif
