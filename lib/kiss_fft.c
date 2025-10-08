/*
 *  Copyright (c) 2003-2010, Mark Borgerding. All rights reserved.
 *  This file is part of KISS FFT - https://github.com/mborgerding/kissfft
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  See COPYING file for more information.
 */


// Bring in declarations from "_kiss_fft_guts.h".
#include "_kiss_fft_guts.h"
/* The guts header contains all the multiplication and addition macros that are defined for
 fixed or floating point complex numbers.  It also declares the kf_ internal functions.
 */
// Start declaration for static void kf_bfly2 with arguments listed on subsequent lines.
static void kf_bfly2(
        // Specify parameter or initializer kiss_fft_cpx * Fout.
        kiss_fft_cpx * Fout,
        // Declare constant const size_t fstride,.
        const size_t fstride,
        // Declare constant const kiss_fft_cfg st,.
        const kiss_fft_cfg st,
        // Execute int m.
        int m
        // Execute ).
        )
// Open a new scope block.
{
    // Declare kiss_fft_cpx * Fout2.
    kiss_fft_cpx * Fout2;
    // Assign kiss_fft_cpx * tw1 to st->twiddles.
    kiss_fft_cpx * tw1 = st->twiddles;
    // Declare kiss_fft_cpx t.
    kiss_fft_cpx t;
    // Assign Fout2 to Fout + m.
    Fout2 = Fout + m;
    // Start the body of a do-while loop.
    do{
        // Call C_FIXDIV with arguments (*Fout,2); C_FIXDIV(*Fout2,2).
        C_FIXDIV(*Fout,2); C_FIXDIV(*Fout2,2);

        // Call C_MUL with arguments (t,  *Fout2 , *tw1).
        C_MUL (t,  *Fout2 , *tw1);
        // Assign tw1 + to fstride.
        tw1 += fstride;
        // Call C_SUB with arguments (*Fout2 ,  *Fout , t).
        C_SUB( *Fout2 ,  *Fout , t );
        // Call C_ADDTO with arguments (*Fout ,  t).
        C_ADDTO( *Fout ,  t );
        // Execute statement ++Fout2.
        ++Fout2;
        // Execute statement ++Fout.
        ++Fout;
    // Close the current scope and emit the trailing comment.
    }while (--m);
// Close the current scope block.
}

// Start declaration for static void kf_bfly4 with arguments listed on subsequent lines.
static void kf_bfly4(
        // Specify parameter or initializer kiss_fft_cpx * Fout.
        kiss_fft_cpx * Fout,
        // Declare constant const size_t fstride,.
        const size_t fstride,
        // Declare constant const kiss_fft_cfg st,.
        const kiss_fft_cfg st,
        // Declare constant const size_t m.
        const size_t m
        // Execute ).
        )
// Open a new scope block.
{
    // Declare kiss_fft_cpx *tw1,*tw2,*tw3.
    kiss_fft_cpx *tw1,*tw2,*tw3;
    // Declare kiss_fft_cpx scratch[6].
    kiss_fft_cpx scratch[6];
    // Assign size_t k to m.
    size_t k=m;
    // Declare constant const size_t m2.
    const size_t m2=2*m;
    // Declare constant const size_t m3.
    const size_t m3=3*m;


    // Assign tw3 to tw2 = tw1 = st->twiddles.
    tw3 = tw2 = tw1 = st->twiddles;

    // Start the body of a do-while loop.
    do {
        // Call C_FIXDIV with arguments (*Fout,4); C_FIXDIV(Fout[m],4); C_FIXDIV(Fout[m2],4); C_FIXDIV(Fout[m3],4).
        C_FIXDIV(*Fout,4); C_FIXDIV(Fout[m],4); C_FIXDIV(Fout[m2],4); C_FIXDIV(Fout[m3],4);

        // Call C_MUL with arguments (scratch[0],Fout[m] , *tw1).
        C_MUL(scratch[0],Fout[m] , *tw1 );
        // Call C_MUL with arguments (scratch[1],Fout[m2] , *tw2).
        C_MUL(scratch[1],Fout[m2] , *tw2 );
        // Call C_MUL with arguments (scratch[2],Fout[m3] , *tw3).
        C_MUL(scratch[2],Fout[m3] , *tw3 );

        // Call C_SUB with arguments (scratch[5] , *Fout, scratch[1]).
        C_SUB( scratch[5] , *Fout, scratch[1] );
        // Call C_ADDTO with arguments (*Fout, scratch[1]).
        C_ADDTO(*Fout, scratch[1]);
        // Call C_ADD with arguments (scratch[3] , scratch[0] , scratch[2]).
        C_ADD( scratch[3] , scratch[0] , scratch[2] );
        // Call C_SUB with arguments (scratch[4] , scratch[0] , scratch[2]).
        C_SUB( scratch[4] , scratch[0] , scratch[2] );
        // Call C_SUB with arguments (Fout[m2], *Fout, scratch[3]).
        C_SUB( Fout[m2], *Fout, scratch[3] );
        // Assign tw1 + to fstride.
        tw1 += fstride;
        // Assign tw2 + to fstride*2.
        tw2 += fstride*2;
        // Assign tw3 + to fstride*3.
        tw3 += fstride*3;
        // Call C_ADDTO with arguments (*Fout , scratch[3]).
        C_ADDTO( *Fout , scratch[3] );

        // Branch when condition (st->inverse) evaluates to true.
        if(st->inverse) {
            // Assign Fout[m].r to scratch[5].r - scratch[4].i.
            Fout[m].r = scratch[5].r - scratch[4].i;
            // Assign Fout[m].i to scratch[5].i + scratch[4].r.
            Fout[m].i = scratch[5].i + scratch[4].r;
            // Assign Fout[m3].r to scratch[5].r + scratch[4].i.
            Fout[m3].r = scratch[5].r + scratch[4].i;
            // Assign Fout[m3].i to scratch[5].i - scratch[4].r.
            Fout[m3].i = scratch[5].i - scratch[4].r;
        // Close the current scope and emit the trailing comment.
        }else{
            // Assign Fout[m].r to scratch[5].r + scratch[4].i.
            Fout[m].r = scratch[5].r + scratch[4].i;
            // Assign Fout[m].i to scratch[5].i - scratch[4].r.
            Fout[m].i = scratch[5].i - scratch[4].r;
            // Assign Fout[m3].r to scratch[5].r - scratch[4].i.
            Fout[m3].r = scratch[5].r - scratch[4].i;
            // Assign Fout[m3].i to scratch[5].i + scratch[4].r.
            Fout[m3].i = scratch[5].i + scratch[4].r;
        // Close the current scope block.
        }
        // Execute statement ++Fout.
        ++Fout;
    // Close the current scope and emit the trailing comment.
    }while(--k);
// Close the current scope block.
}

// Start declaration for static void kf_bfly3 with arguments listed on subsequent lines.
static void kf_bfly3(
         // Specify parameter or initializer kiss_fft_cpx * Fout.
         kiss_fft_cpx * Fout,
         // Declare constant const size_t fstride,.
         const size_t fstride,
         // Declare constant const kiss_fft_cfg st,.
         const kiss_fft_cfg st,
         // Execute size_t m.
         size_t m
         // Execute ).
         )
// Open a new scope block.
{
     // Assign size_t k to m.
     size_t k=m;
     // Declare constant const size_t m2.
     const size_t m2 = 2*m;
     // Declare kiss_fft_cpx *tw1,*tw2.
     kiss_fft_cpx *tw1,*tw2;
     // Declare kiss_fft_cpx scratch[5].
     kiss_fft_cpx scratch[5];
     // Declare kiss_fft_cpx epi3.
     kiss_fft_cpx epi3;
     // Assign epi3 to st->twiddles[fstride*m].
     epi3 = st->twiddles[fstride*m];

     // Assign tw1 to tw2=st->twiddles.
     tw1=tw2=st->twiddles;

     // Start the body of a do-while loop.
     do{
         // Call C_FIXDIV with arguments (*Fout,3); C_FIXDIV(Fout[m],3); C_FIXDIV(Fout[m2],3).
         C_FIXDIV(*Fout,3); C_FIXDIV(Fout[m],3); C_FIXDIV(Fout[m2],3);

         // Call C_MUL with arguments (scratch[1],Fout[m] , *tw1).
         C_MUL(scratch[1],Fout[m] , *tw1);
         // Call C_MUL with arguments (scratch[2],Fout[m2] , *tw2).
         C_MUL(scratch[2],Fout[m2] , *tw2);

         // Call C_ADD with arguments (scratch[3],scratch[1],scratch[2]).
         C_ADD(scratch[3],scratch[1],scratch[2]);
         // Call C_SUB with arguments (scratch[0],scratch[1],scratch[2]).
         C_SUB(scratch[0],scratch[1],scratch[2]);
         // Assign tw1 + to fstride.
         tw1 += fstride;
         // Assign tw2 + to fstride*2.
         tw2 += fstride*2;

         // Assign Fout[m].r to Fout->r - HALF_OF(scratch[3].r).
         Fout[m].r = Fout->r - HALF_OF(scratch[3].r);
         // Assign Fout[m].i to Fout->i - HALF_OF(scratch[3].i).
         Fout[m].i = Fout->i - HALF_OF(scratch[3].i);

         // Call C_MULBYSCALAR with arguments (scratch[0] , epi3.i).
         C_MULBYSCALAR( scratch[0] , epi3.i );

         // Call C_ADDTO with arguments (*Fout,scratch[3]).
         C_ADDTO(*Fout,scratch[3]);

         // Assign Fout[m2].r to Fout[m].r + scratch[0].i.
         Fout[m2].r = Fout[m].r + scratch[0].i;
         // Assign Fout[m2].i to Fout[m].i - scratch[0].r.
         Fout[m2].i = Fout[m].i - scratch[0].r;

         // Assign Fout[m].r - to scratch[0].i.
         Fout[m].r -= scratch[0].i;
         // Assign Fout[m].i + to scratch[0].r.
         Fout[m].i += scratch[0].r;

         // Execute statement ++Fout.
         ++Fout;
     // Close the current scope and emit the trailing comment.
     }while(--k);
// Close the current scope block.
}

// Start declaration for static void kf_bfly5 with arguments listed on subsequent lines.
static void kf_bfly5(
        // Specify parameter or initializer kiss_fft_cpx * Fout.
        kiss_fft_cpx * Fout,
        // Declare constant const size_t fstride,.
        const size_t fstride,
        // Declare constant const kiss_fft_cfg st,.
        const kiss_fft_cfg st,
        // Execute int m.
        int m
        // Execute ).
        )
// Open a new scope block.
{
    // Declare kiss_fft_cpx *Fout0,*Fout1,*Fout2,*Fout3,*Fout4.
    kiss_fft_cpx *Fout0,*Fout1,*Fout2,*Fout3,*Fout4;
    // Declare int u.
    int u;
    // Declare kiss_fft_cpx scratch[13].
    kiss_fft_cpx scratch[13];
    // Assign kiss_fft_cpx * twiddles to st->twiddles.
    kiss_fft_cpx * twiddles = st->twiddles;
    // Declare kiss_fft_cpx *tw.
    kiss_fft_cpx *tw;
    // Declare kiss_fft_cpx ya,yb.
    kiss_fft_cpx ya,yb;
    // Assign ya to twiddles[fstride*m].
    ya = twiddles[fstride*m];
    // Assign yb to twiddles[fstride*2*m].
    yb = twiddles[fstride*2*m];

    // Assign Fout0 to Fout.
    Fout0=Fout;
    // Assign Fout1 to Fout0+m.
    Fout1=Fout0+m;
    // Assign Fout2 to Fout0+2*m.
    Fout2=Fout0+2*m;
    // Assign Fout3 to Fout0+3*m.
    Fout3=Fout0+3*m;
    // Assign Fout4 to Fout0+4*m.
    Fout4=Fout0+4*m;

    // Assign tw to st->twiddles.
    tw=st->twiddles;
    // Iterate with loop parameters ( u=0; u<m; ++u ).
    for ( u=0; u<m; ++u ) {
        // Call C_FIXDIV with arguments (*Fout0,5); C_FIXDIV( *Fout1,5); C_FIXDIV( *Fout2,5); C_FIXDIV( *Fout3,5); C_FIXDIV( *Fout4,5).
        C_FIXDIV( *Fout0,5); C_FIXDIV( *Fout1,5); C_FIXDIV( *Fout2,5); C_FIXDIV( *Fout3,5); C_FIXDIV( *Fout4,5);
        // Assign scratch[0] to *Fout0.
        scratch[0] = *Fout0;

        // Call C_MUL with arguments (scratch[1] ,*Fout1, tw[u*fstride]).
        C_MUL(scratch[1] ,*Fout1, tw[u*fstride]);
        // Call C_MUL with arguments (scratch[2] ,*Fout2, tw[2*u*fstride]).
        C_MUL(scratch[2] ,*Fout2, tw[2*u*fstride]);
        // Call C_MUL with arguments (scratch[3] ,*Fout3, tw[3*u*fstride]).
        C_MUL(scratch[3] ,*Fout3, tw[3*u*fstride]);
        // Call C_MUL with arguments (scratch[4] ,*Fout4, tw[4*u*fstride]).
        C_MUL(scratch[4] ,*Fout4, tw[4*u*fstride]);

        // Call C_ADD with arguments (scratch[7],scratch[1],scratch[4]).
        C_ADD( scratch[7],scratch[1],scratch[4]);
        // Call C_SUB with arguments (scratch[10],scratch[1],scratch[4]).
        C_SUB( scratch[10],scratch[1],scratch[4]);
        // Call C_ADD with arguments (scratch[8],scratch[2],scratch[3]).
        C_ADD( scratch[8],scratch[2],scratch[3]);
        // Call C_SUB with arguments (scratch[9],scratch[2],scratch[3]).
        C_SUB( scratch[9],scratch[2],scratch[3]);

        // Assign Fout0->r + to scratch[7].r + scratch[8].r.
        Fout0->r += scratch[7].r + scratch[8].r;
        // Assign Fout0->i + to scratch[7].i + scratch[8].i.
        Fout0->i += scratch[7].i + scratch[8].i;

        // Assign scratch[5].r to scratch[0].r + S_MUL(scratch[7].r,ya.r) + S_MUL(scratch[8].r,yb.r).
        scratch[5].r = scratch[0].r + S_MUL(scratch[7].r,ya.r) + S_MUL(scratch[8].r,yb.r);
        // Assign scratch[5].i to scratch[0].i + S_MUL(scratch[7].i,ya.r) + S_MUL(scratch[8].i,yb.r).
        scratch[5].i = scratch[0].i + S_MUL(scratch[7].i,ya.r) + S_MUL(scratch[8].i,yb.r);

        // Assign scratch[6].r to S_MUL(scratch[10].i,ya.i) + S_MUL(scratch[9].i,yb.i).
        scratch[6].r =  S_MUL(scratch[10].i,ya.i) + S_MUL(scratch[9].i,yb.i);
        // Assign scratch[6].i to -S_MUL(scratch[10].r,ya.i) - S_MUL(scratch[9].r,yb.i).
        scratch[6].i = -S_MUL(scratch[10].r,ya.i) - S_MUL(scratch[9].r,yb.i);

        // Call C_SUB with arguments (*Fout1,scratch[5],scratch[6]).
        C_SUB(*Fout1,scratch[5],scratch[6]);
        // Call C_ADD with arguments (*Fout4,scratch[5],scratch[6]).
        C_ADD(*Fout4,scratch[5],scratch[6]);

        // Assign scratch[11].r to scratch[0].r + S_MUL(scratch[7].r,yb.r) + S_MUL(scratch[8].r,ya.r).
        scratch[11].r = scratch[0].r + S_MUL(scratch[7].r,yb.r) + S_MUL(scratch[8].r,ya.r);
        // Assign scratch[11].i to scratch[0].i + S_MUL(scratch[7].i,yb.r) + S_MUL(scratch[8].i,ya.r).
        scratch[11].i = scratch[0].i + S_MUL(scratch[7].i,yb.r) + S_MUL(scratch[8].i,ya.r);
        // Assign scratch[12].r to - S_MUL(scratch[10].i,yb.i) + S_MUL(scratch[9].i,ya.i).
        scratch[12].r = - S_MUL(scratch[10].i,yb.i) + S_MUL(scratch[9].i,ya.i);
        // Assign scratch[12].i to S_MUL(scratch[10].r,yb.i) - S_MUL(scratch[9].r,ya.i).
        scratch[12].i = S_MUL(scratch[10].r,yb.i) - S_MUL(scratch[9].r,ya.i);

        // Call C_ADD with arguments (*Fout2,scratch[11],scratch[12]).
        C_ADD(*Fout2,scratch[11],scratch[12]);
        // Call C_SUB with arguments (*Fout3,scratch[11],scratch[12]).
        C_SUB(*Fout3,scratch[11],scratch[12]);

        // Execute statement ++Fout0;++Fout1;++Fout2;++Fout3;++Fout4.
        ++Fout0;++Fout1;++Fout2;++Fout3;++Fout4;
    // Close the current scope block.
    }
// Close the current scope block.
}

/* perform the butterfly for one stage of a mixed radix FFT */
// Start declaration for static void kf_bfly_generic with arguments listed on subsequent lines.
static void kf_bfly_generic(
        // Specify parameter or initializer kiss_fft_cpx * Fout.
        kiss_fft_cpx * Fout,
        // Declare constant const size_t fstride,.
        const size_t fstride,
        // Declare constant const kiss_fft_cfg st,.
        const kiss_fft_cfg st,
        // Specify parameter or initializer int m.
        int m,
        // Execute int p.
        int p
        // Execute ).
        )
// Open a new scope block.
{
    // Declare int u,k,q1,q.
    int u,k,q1,q;
    // Assign kiss_fft_cpx * twiddles to st->twiddles.
    kiss_fft_cpx * twiddles = st->twiddles;
    // Declare kiss_fft_cpx t.
    kiss_fft_cpx t;
    // Assign int Norig to st->nfft.
    int Norig = st->nfft;

    // Assign kiss_fft_cpx * scratch to (kiss_fft_cpx*)KISS_FFT_TMP_ALLOC(sizeof(kiss_fft_cpx)*p).
    kiss_fft_cpx * scratch = (kiss_fft_cpx*)KISS_FFT_TMP_ALLOC(sizeof(kiss_fft_cpx)*p);

    // Iterate with loop parameters ( u=0; u<m; ++u ).
    for ( u=0; u<m; ++u ) {
        // Assign k to u.
        k=u;
        // Iterate with loop parameters ( q1=0 ; q1<p ; ++q1 ).
        for ( q1=0 ; q1<p ; ++q1 ) {
            // Assign scratch[q1] to Fout[ k  ].
            scratch[q1] = Fout[ k  ];
            // Call C_FIXDIV with arguments (scratch[q1],p).
            C_FIXDIV(scratch[q1],p);
            // Assign k + to m.
            k += m;
        // Close the current scope block.
        }

        // Assign k to u.
        k=u;
        // Iterate with loop parameters ( q1=0 ; q1<p ; ++q1 ).
        for ( q1=0 ; q1<p ; ++q1 ) {
            // Assign int twidx to 0.
            int twidx=0;
            // Assign Fout[ k ] to scratch[0].
            Fout[ k ] = scratch[0];
            // Iterate with loop parameters (q=1;q<p;++q ).
            for (q=1;q<p;++q ) {
                // Assign twidx + to fstride * k.
                twidx += fstride * k;
                // Branch when condition (twidx>=Norig) evaluates to true.
                if (twidx>=Norig) twidx-=Norig;
                // Call C_MUL with arguments (t,scratch[q] , twiddles[twidx]).
                C_MUL(t,scratch[q] , twiddles[twidx] );
                // Call C_ADDTO with arguments (Fout[ k ] ,t).
                C_ADDTO( Fout[ k ] ,t);
            // Close the current scope block.
            }
            // Assign k + to m.
            k += m;
        // Close the current scope block.
        }
    // Close the current scope block.
    }
    // Call KISS_FFT_TMP_FREE with arguments (scratch).
    KISS_FFT_TMP_FREE(scratch);
// Close the current scope block.
}

// Execute static.
static
// Start declaration for void kf_work with arguments listed on subsequent lines.
void kf_work(
        // Specify parameter or initializer kiss_fft_cpx * Fout.
        kiss_fft_cpx * Fout,
        // Declare constant const kiss_fft_cpx * f,.
        const kiss_fft_cpx * f,
        // Declare constant const size_t fstride,.
        const size_t fstride,
        // Specify parameter or initializer int in_stride.
        int in_stride,
        // Specify parameter or initializer int * factors.
        int * factors,
        // Declare constant const kiss_fft_cfg st.
        const kiss_fft_cfg st
        // Execute ).
        )
// Open a new scope block.
{
    // Assign kiss_fft_cpx * Fout_beg to Fout.
    kiss_fft_cpx * Fout_beg=Fout;
    // Declare constant const int p.
    const int p=*factors++; /* the radix  */
    // Declare constant const int m.
    const int m=*factors++; /* stage's fft length/p */
    // Declare constant const kiss_fft_cpx * Fout_end.
    const kiss_fft_cpx * Fout_end = Fout + p*m;

// Only compile the following section when _OPENMP is defined.
#ifdef _OPENMP
    // use openmp extensions at the
    // top-level (not recursive)
    // Branch when condition (fstride==1 && p<=5 && m!=1) evaluates to true.
    if (fstride==1 && p<=5 && m!=1)
    // Open a new scope block.
    {
        // Declare int k.
        int k;

        // execute the p different work units in different threads
// Execute #       pragma omp parallel for.
#       pragma omp parallel for
        // Iterate with loop parameters (k=0;k<p;++k).
        for (k=0;k<p;++k)
            // Call kf_work with arguments (Fout +k*m, f+ fstride*in_stride*k,fstride*p,in_stride,factors,st).
            kf_work( Fout +k*m, f+ fstride*in_stride*k,fstride*p,in_stride,factors,st);
        // all threads have joined by this point

        // Select behavior based on (p).
        switch (p) {
            // Handle switch label case 2.
            case 2: kf_bfly2(Fout,fstride,st,m); break;
            // Handle switch label case 3.
            case 3: kf_bfly3(Fout,fstride,st,m); break;
            // Handle switch label case 4.
            case 4: kf_bfly4(Fout,fstride,st,m); break;
            // Handle switch label case 5.
            case 5: kf_bfly5(Fout,fstride,st,m); break;
            // Handle default switch label.
            default: kf_bfly_generic(Fout,fstride,st,m,p); break;
        // Close the current scope block.
        }
        // Return to the caller.
        return;
    // Close the current scope block.
    }
// Close the preceding conditional compilation block.
#endif

    // Branch when condition (m==1) evaluates to true.
    if (m==1) {
        // Start the body of a do-while loop.
        do{
            // Assign *Fout to *f.
            *Fout = *f;
            // Assign f + to fstride*in_stride.
            f += fstride*in_stride;
        // Close the current scope and emit the trailing comment.
        }while(++Fout != Fout_end );
    // Close the current scope and emit the trailing comment.
    }else{
        // Start the body of a do-while loop.
        do{
            // recursive call:
            // DFT of size m*p performed by doing
            // p instances of smaller DFTs of size m,
            // each one takes a decimated version of the input
            // Call kf_work with arguments (Fout , f, fstride*p, in_stride, factors,st).
            kf_work( Fout , f, fstride*p, in_stride, factors,st);
            // Assign f + to fstride*in_stride.
            f += fstride*in_stride;
        // Close the current scope and emit the trailing comment.
        }while( (Fout += m) != Fout_end );
    // Close the current scope block.
    }

    // Assign Fout to Fout_beg.
    Fout=Fout_beg;

    // recombine the p smaller DFTs
    // Select behavior based on (p).
    switch (p) {
        // Handle switch label case 2.
        case 2: kf_bfly2(Fout,fstride,st,m); break;
        // Handle switch label case 3.
        case 3: kf_bfly3(Fout,fstride,st,m); break;
        // Handle switch label case 4.
        case 4: kf_bfly4(Fout,fstride,st,m); break;
        // Handle switch label case 5.
        case 5: kf_bfly5(Fout,fstride,st,m); break;
        // Handle default switch label.
        default: kf_bfly_generic(Fout,fstride,st,m,p); break;
    // Close the current scope block.
    }
// Close the current scope block.
}

/*  facbuf is populated by p1,m1,p2,m2, ...
    where
    p[i] * m[i] = m[i-1]
    m0 = n                  */
// Execute static.
static
// Define the r function.
void kf_factor(int n,int * facbuf)
// Open a new scope block.
{
    // Assign int p to 4.
    int p=4;
    // Start the body of a do-while loop.
    double floor_sqrt;
    // Assign floor_sqrt to floor( sqrt((double)n) ).
    floor_sqrt = floor( sqrt((double)n) );

    /*factor out powers of 4, powers of 2, then any remaining primes */
    // Start the body of a do-while loop.
    do {
        // Repeat while (n % p) remains true.
        while (n % p) {
            // Select behavior based on (p).
            switch (p) {
                // Handle switch label case 4.
                case 4: p = 2; break;
                // Handle switch label case 2.
                case 2: p = 3; break;
                // Handle default switch label.
                default: p += 2; break;
            // Close the current scope block.
            }
            // Branch when condition (p > floor_sqrt) evaluates to true.
            if (p > floor_sqrt)
                // Execute p = n;          /* no more factors, skip to end */.
                p = n;          /* no more factors, skip to end */
        // Close the current scope block.
        }
        // Assign n / to p.
        n /= p;
        // Assign *facbuf++ to p.
        *facbuf++ = p;
        // Assign *facbuf++ to n.
        *facbuf++ = n;
    // Close the current scope and emit the trailing comment.
    } while (n > 1);
// Close the current scope block.
}

/*
 *
 * User-callable function to allocate all necessary storage space for the fft.
 *
 * The return value is a contiguous block of memory, allocated with malloc.  As such,
 * It can be freed with free(), rather than a kiss_fft-specific function.
 * */
// Define the c function.
kiss_fft_cfg kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem )
// Open a new scope block.
{
    // Assign kiss_fft_cfg st to NULL.
    kiss_fft_cfg st=NULL;
    // Evaluate expression size_t memneeded = sizeof(struct kiss_fft_state).
    size_t memneeded = sizeof(struct kiss_fft_state)
        // Execute + sizeof(kiss_fft_cpx)*(nfft-1); /* twiddle factors*/.
        + sizeof(kiss_fft_cpx)*(nfft-1); /* twiddle factors*/

    // Branch when condition ( lenmem==NULL ) evaluates to true.
    if ( lenmem==NULL ) {
        // Assign st to ( kiss_fft_cfg)KISS_FFT_MALLOC( memneeded ).
        st = ( kiss_fft_cfg)KISS_FFT_MALLOC( memneeded );
    // Close the current scope and emit the trailing comment.
    }else{
        // Branch when condition (mem != NULL && *lenmem >= memneeded) evaluates to true.
        if (mem != NULL && *lenmem >= memneeded)
            // Assign st to (kiss_fft_cfg)mem.
            st = (kiss_fft_cfg)mem;
        // Assign *lenmem to memneeded.
        *lenmem = memneeded;
    // Close the current scope block.
    }
    // Branch when condition (st) evaluates to true.
    if (st) {
        // Declare int i.
        int i;
        // Assign st->nfft to nfft.
        st->nfft=nfft;
        // Assign st->inverse to inverse_fft.
        st->inverse = inverse_fft;

        // Iterate with loop parameters (i=0;i<nfft;++i).
        for (i=0;i<nfft;++i) {
            // Declare constant const double pi.
            const double pi=3.141592653589793238462643383279502884197169399375105820974944;
            // Start the body of a do-while loop.
            double phase = -2*pi*i / nfft;
            // Branch when condition (st->inverse) evaluates to true.
            if (st->inverse)
                // Assign phase * to -1.
                phase *= -1;
            // Call kf_cexp with arguments (st->twiddles+i, phase).
            kf_cexp(st->twiddles+i, phase );
        // Close the current scope block.
        }

        // Call kf_factor with arguments (nfft,st->factors).
        kf_factor(nfft,st->factors);
    // Close the current scope block.
    }
    // Return st to the caller.
    return st;
// Close the current scope block.
}


// Define the e function.
void kiss_fft_stride(kiss_fft_cfg st,const kiss_fft_cpx *fin,kiss_fft_cpx *fout,int in_stride)
// Open a new scope block.
{
    // Branch when condition (fin == fout) evaluates to true.
    if (fin == fout) {
        //NOTE: this is not really an in-place FFT algorithm.
        //It just performs an out-of-place FFT into a temp buffer
        // Assign kiss_fft_cpx * tmpbuf to (kiss_fft_cpx*)KISS_FFT_TMP_ALLOC( sizeof(kiss_fft_cpx)*st->nfft).
        kiss_fft_cpx * tmpbuf = (kiss_fft_cpx*)KISS_FFT_TMP_ALLOC( sizeof(kiss_fft_cpx)*st->nfft);
        // Call kf_work with arguments (tmpbuf,fin,1,in_stride, st->factors,st).
        kf_work(tmpbuf,fin,1,in_stride, st->factors,st);
        // Call memcpy with arguments (fout,tmpbuf,sizeof(kiss_fft_cpx)*st->nfft).
        memcpy(fout,tmpbuf,sizeof(kiss_fft_cpx)*st->nfft);
        // Call KISS_FFT_TMP_FREE with arguments (tmpbuf).
        KISS_FFT_TMP_FREE(tmpbuf);
    // Close the current scope and emit the trailing comment.
    }else{
        // Call kf_work with arguments (fout, fin, 1,in_stride, st->factors,st).
        kf_work( fout, fin, 1,in_stride, st->factors,st );
    // Close the current scope block.
    }
// Close the current scope block.
}

// Define the t function.
void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout)
// Open a new scope block.
{
    // Call kiss_fft_stride with arguments (cfg,fin,fout,1).
    kiss_fft_stride(cfg,fin,fout,1);
// Close the current scope block.
}


// Define the p function.
void kiss_fft_cleanup(void)
// Open a new scope block.
{
    // nothing needed any more
// Close the current scope block.
}

// Define the e function.
int kiss_fft_next_fast_size(int n)
// Open a new scope block.
{
    // Repeat while (1) remains true.
    while(1) {
        // Assign int m to n.
        int m=n;
        // Repeat while ( (m%2) == 0 ) remains true.
        while ( (m%2) == 0 ) m/=2;
        // Repeat while ( (m%3) == 0 ) remains true.
        while ( (m%3) == 0 ) m/=3;
        // Repeat while ( (m%5) == 0 ) remains true.
        while ( (m%5) == 0 ) m/=5;
        // Branch when condition (m<=1) evaluates to true.
        if (m<=1)
            // Exit the nearest enclosing loop or switch.
            break; /* n is completely factorable by twos, threes, and fives */
        // Increment n.
        n++;
    // Close the current scope block.
    }
    // Return n to the caller.
    return n;
// Close the current scope block.
}
