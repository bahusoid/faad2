/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: cfft.c,v 1.17 2003/11/04 21:43:30 menno Exp $
**/

/*
 * Algorithmically based on Fortran-77 FFTPACK
 * by Paul N. Swarztrauber(Version 4, 1985).
 *
 * Does even sized fft only
 */

/* isign is +1 for backward and -1 for forward transforms */

#include "common.h"
#include "structs.h"

#include <stdlib.h>

#include "cfft.h"
#include "cfft_tab.h"


/*----------------------------------------------------------------------
   passf2, passf3, passf4, passf5. Complex FFT passes fwd and bwd.
  ----------------------------------------------------------------------*/

static void passf2(const uint16_t ido, const uint16_t l1, const complex_t *cc,
                   complex_t *ch, const complex_t *wa, const int8_t isign)
{
    uint16_t i, k, ah, ac;

    if (ido == 1)
    {
        for (k = 0; k < l1; k++)
        {
            ah = 2*k;
            ac = 4*k;

            RE(ch[ah])    = RE(cc[ac]) + RE(cc[ac+1]);
            RE(ch[ah+l1]) = RE(cc[ac]) - RE(cc[ac+1]);
            IM(ch[ah])    = IM(cc[ac]) + IM(cc[ac+1]);
            IM(ch[ah+l1]) = IM(cc[ac]) - IM(cc[ac+1]);
        }
    } else {
        for (k = 0; k < l1; k++)
        {
            ah = k*ido;
            ac = 2*k*ido;

            for (i = 0; i < ido; i++)
            {
                complex_t t2;

                RE(ch[ah+i]) = RE(cc[ac+i]) + RE(cc[ac+i+ido]);
                RE(t2)       = RE(cc[ac+i]) - RE(cc[ac+i+ido]);

                IM(ch[ah+i]) = IM(cc[ac+i]) + IM(cc[ac+i+ido]);
                IM(t2)       = IM(cc[ac+i]) - IM(cc[ac+i+ido]);

                RE(ch[ah+i+l1*ido]) = MUL_R_C(RE(t2),RE(wa[i])) - MUL_R_C(IM(t2),IM(wa[i]))*isign;
                IM(ch[ah+i+l1*ido]) = MUL_R_C(IM(t2),RE(wa[i])) + MUL_R_C(RE(t2),IM(wa[i]))*isign;
            }
        }
    }
}


static void passf3(const uint16_t ido, const uint16_t l1, const complex_t *cc,
                   complex_t *ch, const complex_t *wa1, const complex_t *wa2,
                   const int8_t isign)
{
    static real_t taur = COEF_CONST(-0.5);
    static real_t taui = COEF_CONST(0.866025403784439);
    uint16_t i, k, ac, ah;
    complex_t c2, c3, d2, d3, t2;

    if (ido == 1)
    {
        for (k = 0; k < l1; k++)
        {
            ac = 3*k+1;
            ah = k;

            RE(t2) = RE(cc[ac]) + RE(cc[ac+1]);
            IM(t2) = IM(cc[ac]) + IM(cc[ac+1]);
            RE(c2) = RE(cc[ac-1]) + MUL_R_C(RE(t2),taur);
            IM(c2) = IM(cc[ac-1]) + MUL_R_C(IM(t2),taur);

            RE(ch[ah]) = RE(cc[ac-1]) + RE(t2);
            IM(ch[ah]) = IM(cc[ac-1]) + IM(t2);

            RE(c3) = MUL_R_C((RE(cc[ac]) - RE(cc[ac+1])), taui)*isign;
            IM(c3) = MUL_R_C((IM(cc[ac]) - IM(cc[ac+1])), taui)*isign;

            RE(ch[ah+l1]) = RE(c2) - IM(c3);
            IM(ch[ah+l1]) = IM(c2) + RE(c3);
            RE(ch[ah+2*l1]) = RE(c2) + IM(c3);
            IM(ch[ah+2*l1]) = IM(c2) - RE(c3);
        }
    } else {
        for (k = 0; k < l1; k++)
        {
            for (i = 0; i < ido; i++)
            {
                ac = i + (3*k+1)*ido;
                ah = i + k * ido;

                RE(t2) = RE(cc[ac]) + RE(cc[ac+ido]);
                RE(c2) = RE(cc[ac-ido]) + MUL_R_C(RE(t2),taur);
                IM(t2) = IM(cc[ac]) + IM(cc[ac+ido]);
                IM(c2) = IM(cc[ac-ido]) + MUL_R_C(IM(t2),taur);

                RE(ch[ah]) = RE(cc[ac-ido]) + RE(t2);
                IM(ch[ah]) = IM(cc[ac-ido]) + IM(t2);

                RE(c3) = MUL_R_C((RE(cc[ac]) - RE(cc[ac+ido])), taui)*isign;
                IM(c3) = MUL_R_C((IM(cc[ac]) - IM(cc[ac+ido])), taui)*isign;

                RE(d2) = RE(c2) - IM(c3);
                IM(d3) = IM(c2) - RE(c3);
                RE(d3) = RE(c2) + IM(c3);
                IM(d2) = IM(c2) + RE(c3);

                RE(ch[ah+l1*ido]) = MUL_R_C(RE(d2),RE(wa1[i])) - MUL_R_C(IM(d2),IM(wa1[i]))*isign;
                IM(ch[ah+l1*ido]) = MUL_R_C(IM(d2),RE(wa1[i])) + MUL_R_C(RE(d2),IM(wa1[i]))*isign;
                RE(ch[ah+l1*2*ido]) = MUL_R_C(RE(d3),RE(wa2[i])) - MUL_R_C(IM(d3),IM(wa2[i]))*isign;
                IM(ch[ah+l1*2*ido]) = MUL_R_C(IM(d3),RE(wa2[i])) + MUL_R_C(RE(d3),IM(wa2[i]))*isign;
            }
        }
    }
}

#if 0
typedef real_t simd_complex_t[4];

/*
  complex_add_sub(c1, c2, a1, a2);
  complex_mult(a1, c1, w0);
  complex_mult(a2, c2, w2);
*/
static INLINE void complex_func(simd_complex_t a1, simd_complex_t a2,
                                const simd_complex_t z1, const simd_complex_t z2,
                                const simd_complex_t w1, const simd_complex_t w2)
{
    __asm {
        mov eax, a1
        mov ebx, a2
        movups xmm0, [eax]
        movups xmm2, [ebx]
        movups xmm4, [eax]
        addps xmm0, xmm2   ; xmm0 = c1
        subps xmm4, xmm2   ; xmm4 = c2


        ; complex mult
        mov ecx, w1
        movups xmm1, [ecx]
        movups xmm2, xmm0
        movups xmm3, xmm1

        mulps xmm0, xmm1

        shufps xmm2, xmm2, 0xB1
        shufps xmm0, xmm0, 0xD8

        mulps xmm2, xmm3

        movhlps xmm1, xmm0
        shufps xmm2, xmm2, 0xD8

        subps xmm0, xmm1
        movhlps xmm3, xmm2
        addps xmm2, xmm3

        unpcklps xmm0, xmm2
        movups [eax], xmm0

        ; complex mult
        mov ecx, w2
        movups xmm1, [ecx]
        movups xmm2, xmm4
        movups xmm3, xmm1

        mulps xmm4, xmm1

        shufps xmm2, xmm2, 0xB1
        shufps xmm4, xmm4, 0xD8

        mulps xmm2, xmm3

        movhlps xmm1, xmm4
        shufps xmm2, xmm2, 0xD8

        subps xmm4, xmm1
        movhlps xmm3, xmm2
        addps xmm2, xmm3

        unpcklps xmm4, xmm2
        movups [ebx], xmm4
    }
}

/* complex a = z1*z2 */
static INLINE void complex_mult(simd_complex_t a, const simd_complex_t z1, const simd_complex_t z2)
{
#if 0
    a[0] = MUL_R_C(z1[0],z2[0]) - MUL_R_C(z1[1],z2[1]);
    a[1] = MUL_R_C(z1[1],z2[0]) + MUL_R_C(z1[0],z2[1]);

    a[2] = MUL_R_C(z1[2],z2[2]) - MUL_R_C(z1[3],z2[3]);
    a[3] = MUL_R_C(z1[3],z2[2]) + MUL_R_C(z1[2],z2[3]);
#else
    __asm {
        mov eax, z1
        mov ecx, z2
        mov edx, a

        movups xmm0, [eax]
        movups xmm1, [ecx]
        movaps xmm2, xmm0
        movaps xmm3, xmm1

        mulps xmm0, xmm1

        shufps xmm2, xmm2, 0xB1
        shufps xmm0, xmm0, 0xD8

        mulps xmm2, xmm3

        movhlps xmm1, xmm0
        shufps xmm2, xmm2, 0xD8

        subps xmm0, xmm1
        movhlps xmm3, xmm2
        addps xmm2, xmm3

        unpcklps xmm0, xmm2

        movups [edx], xmm0
    }
#endif
}

/* complex a = z1+z2 */
static void complex_add(complex_t a, const complex_t z1, const complex_t z2)
{
    RE(a) = RE(z1) + RE(z2);
    IM(a) = IM(z1) + IM(z2);
}

/* complex a = z1-z2 */
static void complex_sub(complex_t a, const complex_t z1, const complex_t z2)
{
    RE(a) = RE(z1) - RE(z2);
    IM(a) = IM(z1) - IM(z2);
}

/* complex a1 = z1+z2; a2 = z1-z2 */
static INLINE void complex_add_sub(simd_complex_t a1, simd_complex_t a2,
                            const simd_complex_t z1, const simd_complex_t z2)
{
#if 0
    a1[0] = z1[0] + z2[0];
    a1[1] = z1[1] + z2[1];
    a1[2] = z1[2] + z2[2];
    a1[3] = z1[3] + z2[3];
    a2[0] = z1[0] - z2[0];
    a2[1] = z1[1] - z2[1];
    a2[2] = z1[2] - z2[2];
    a2[3] = z1[3] - z2[3];
#else
    __asm {
        mov eax, DWORD PTR z1
        mov ebx, DWORD PTR z2
        mov ecx, DWORD PTR a1
        mov edx, DWORD PTR a2
        movups xmm1, [eax]
        movups xmm2, [ebx]
        movups xmm3, [eax]
        addps xmm1, xmm2
        subps xmm3, xmm2
        movups [ecx], xmm1
        movups [edx], xmm3
    }
#endif
}

static void passf4(const uint16_t ido, const uint16_t l1, const complex_t *cc,
                   complex_t *ch, const complex_t *wa1, const complex_t *wa2,
                   const complex_t *wa3, const int8_t isign)
{
    uint16_t i, k, ac, ah;

    if (ido == 1)
    {
        for (k = 0; k < l1; k++)
        {
            complex_t t1, t2, t3, t4;

            ac = 4*k;
            ah = k;

            RE(t2) = RE(cc[ac])   + RE(cc[ac+2]);
            RE(t1) = RE(cc[ac])   - RE(cc[ac+2]);
            IM(t2) = IM(cc[ac])   + IM(cc[ac+2]);
            IM(t1) = IM(cc[ac])   - IM(cc[ac+2]);
            RE(t3) = RE(cc[ac+1]) + RE(cc[ac+3]);
            IM(t4) = RE(cc[ac+1]) - RE(cc[ac+3]);
            IM(t3) = IM(cc[ac+3]) + IM(cc[ac+1]);
            RE(t4) = IM(cc[ac+3]) - IM(cc[ac+1]);

            RE(ch[ah])      = RE(t2) + RE(t3);
            RE(ch[ah+2*l1]) = RE(t2) - RE(t3);

            IM(ch[ah])      = IM(t2) + IM(t3);
            IM(ch[ah+2*l1]) = IM(t2) - IM(t3);

            RE(ch[ah+l1])   = RE(t1) + RE(t4)*isign;
            RE(ch[ah+3*l1]) = RE(t1) - RE(t4)*isign;

            IM(ch[ah+l1])   = IM(t1) + IM(t4)*isign;
            IM(ch[ah+3*l1]) = IM(t1) - IM(t4)*isign;
        }
    } else {
        for (k = 0; k < l1; k++)
        {
            ac = 4*k*ido;
            ah = k*ido;

            for (i = 0; i < ido; i++)
            {
                simd_complex_t c1, c2, t1, t2;
                simd_complex_t w0 = {1,0,0,0};
                simd_complex_t w2;

                w0[2] = wa1[i][0]*isign;
                w0[3] = wa1[i][1]*isign;
                w2[0] = wa2[i][0]*isign;
                w2[1] = wa2[i][1]*isign;
                w2[2] = wa3[i][0]*isign;
                w2[3] = wa3[i][1]*isign;

                t1[0] = RE(cc[ac+i]) + RE(cc[ac+i+2*ido]);
                t1[1] = IM(cc[ac+i]) + IM(cc[ac+i+2*ido]);
                t1[2] = RE(cc[ac+i]) - RE(cc[ac+i+2*ido]);
                t1[3] = IM(cc[ac+i]) - IM(cc[ac+i+2*ido]);
                t2[0] = RE(cc[ac+i+ido]) + RE(cc[ac+i+3*ido]);
                t2[3] = RE(cc[ac+i+ido]) - RE(cc[ac+i+3*ido]);
                t2[1] = IM(cc[ac+i+3*ido]) + IM(cc[ac+i+ido]);
                t2[2] = IM(cc[ac+i+3*ido]) - IM(cc[ac+i+ido]);

                t2[2] *= isign;
                t2[3] *= isign;

#if 0
                complex_add_sub(c1, c2, t1, t2);
                complex_mult(t1, c1, w0);
                complex_mult(t2, c2, w2);
#else
                complex_func(t1, t2, c1, c2, w0, w2);
#endif

                RE(ch[ah+i]) = t1[0];
                IM(ch[ah+i]) = t1[1];
                RE(ch[ah+i+l1*ido]) = t1[2];
                IM(ch[ah+i+l1*ido]) = t1[3];
                RE(ch[ah+i+2*l1*ido]) = t2[0];
                IM(ch[ah+i+2*l1*ido]) = t2[1];
                RE(ch[ah+i+3*l1*ido]) = t2[2];
                IM(ch[ah+i+3*l1*ido]) = t2[3];
            }
        }
    }
}
#else
static void passf4(const uint16_t ido, const uint16_t l1, const complex_t *cc,
                   complex_t *ch, const complex_t *wa1, const complex_t *wa2,
                   const complex_t *wa3, const int8_t isign)
{
    uint16_t i, k, ac, ah;

    if (ido == 1)
    {
        for (k = 0; k < l1; k++)
        {
            complex_t t1, t2, t3, t4;

            ac = 4*k;
            ah = k;

            RE(t2) = RE(cc[ac])   + RE(cc[ac+2]);
            RE(t1) = RE(cc[ac])   - RE(cc[ac+2]);
            IM(t2) = IM(cc[ac])   + IM(cc[ac+2]);
            IM(t1) = IM(cc[ac])   - IM(cc[ac+2]);
            RE(t3) = RE(cc[ac+1]) + RE(cc[ac+3]);
            IM(t4) = RE(cc[ac+1]) - RE(cc[ac+3]);
            IM(t3) = IM(cc[ac+3]) + IM(cc[ac+1]);
            RE(t4) = IM(cc[ac+3]) - IM(cc[ac+1]);

            RE(ch[ah])      = RE(t2) + RE(t3);
            RE(ch[ah+2*l1]) = RE(t2) - RE(t3);

            IM(ch[ah])      = IM(t2) + IM(t3);
            IM(ch[ah+2*l1]) = IM(t2) - IM(t3);

            RE(ch[ah+l1])   = RE(t1) + RE(t4)*isign;
            RE(ch[ah+3*l1]) = RE(t1) - RE(t4)*isign;

            IM(ch[ah+l1])   = IM(t1) + IM(t4)*isign;
            IM(ch[ah+3*l1]) = IM(t1) - IM(t4)*isign;
        }
    } else {
        for (k = 0; k < l1; k++)
        {
            ac = 4*k*ido;
            ah = k*ido;

            for (i = 0; i < ido; i++)
            {
                complex_t c2, c3, c4, t1, t2, t3, t4;

                RE(t2) = RE(cc[ac+i]) + RE(cc[ac+i+2*ido]);
                RE(t1) = RE(cc[ac+i]) - RE(cc[ac+i+2*ido]);
                IM(t2) = IM(cc[ac+i]) + IM(cc[ac+i+2*ido]);
                IM(t1) = IM(cc[ac+i]) - IM(cc[ac+i+2*ido]);
                RE(t3) = RE(cc[ac+i+ido]) + RE(cc[ac+i+3*ido]);
                IM(t4) = RE(cc[ac+i+ido]) - RE(cc[ac+i+3*ido]);
                IM(t3) = IM(cc[ac+i+3*ido]) + IM(cc[ac+i+ido]);
                RE(t4) = IM(cc[ac+i+3*ido]) - IM(cc[ac+i+ido]);

                RE(c2) = RE(t1) + RE(t4)*isign;
                RE(c4) = RE(t1) - RE(t4)*isign;

                IM(c2) = IM(t1) + IM(t4)*isign;
                IM(c4) = IM(t1) - IM(t4)*isign;

                RE(ch[ah+i]) = RE(t2) + RE(t3);
                RE(c3)       = RE(t2) - RE(t3);

                IM(ch[ah+i]) = IM(t2) + IM(t3);
                IM(c3)       = IM(t2) - IM(t3);

                IM(ch[ah+i+l1*ido])   = MUL_R_C(IM(c2),RE(wa1[i])) + MUL_R_C(RE(c2),IM(wa1[i]))*isign;
                RE(ch[ah+i+l1*ido])   = MUL_R_C(RE(c2),RE(wa1[i])) - MUL_R_C(IM(c2),IM(wa1[i]))*isign;
                IM(ch[ah+i+2*l1*ido]) = MUL_R_C(IM(c3),RE(wa2[i])) + MUL_R_C(RE(c3),IM(wa2[i]))*isign;
                RE(ch[ah+i+2*l1*ido]) = MUL_R_C(RE(c3),RE(wa2[i])) - MUL_R_C(IM(c3),IM(wa2[i]))*isign;
                IM(ch[ah+i+3*l1*ido]) = MUL_R_C(IM(c4),RE(wa3[i])) + MUL_R_C(RE(c4),IM(wa3[i]))*isign;
                RE(ch[ah+i+3*l1*ido]) = MUL_R_C(RE(c4),RE(wa3[i])) - MUL_R_C(IM(c4),IM(wa3[i]))*isign;
            }
        }
    }
}
#endif

static void passf5(const uint16_t ido, const uint16_t l1, const complex_t *cc,
                   complex_t *ch, const complex_t *wa1, const complex_t *wa2, const complex_t *wa3,
                   const complex_t *wa4, const int8_t isign)
{
    static real_t tr11 = COEF_CONST(0.309016994374947);
    static real_t ti11 = COEF_CONST(0.951056516295154);
    static real_t tr12 = COEF_CONST(-0.809016994374947);
    static real_t ti12 = COEF_CONST(0.587785252292473);
    uint16_t i, k, ac, ah;
    complex_t c2, c3, c4, c5, d3, d4, d5, d2, t2, t3, t4, t5;

    if (ido == 1)
    {
        for (k = 0; k < l1; k++)
        {
            ac = 5*k + 1;
            ah = k;

            RE(t2) = RE(cc[ac]) + RE(cc[ac+3]);
            IM(t2) = IM(cc[ac]) + IM(cc[ac+3]);
            RE(t3) = RE(cc[ac+1]) + RE(cc[ac+2]);
            IM(t3) = IM(cc[ac+1]) + IM(cc[ac+2]);
            RE(t4) = RE(cc[ac+1]) - RE(cc[ac+2]);
            IM(t4) = IM(cc[ac+1]) - IM(cc[ac+2]);
            RE(t5) = RE(cc[ac]) - RE(cc[ac+3]);
            IM(t5) = IM(cc[ac]) - IM(cc[ac+3]);

            RE(ch[ah]) = RE(cc[ac-1]) + RE(t2) + RE(t3);
            IM(ch[ah]) = IM(cc[ac-1]) + IM(t2) + IM(t3);

            RE(c2) = RE(cc[ac-1]) + MUL_R_C(RE(t2),tr11) + MUL_R_C(RE(t3),tr12);
            IM(c2) = IM(cc[ac-1]) + MUL_R_C(IM(t2),tr11) + MUL_R_C(IM(t3),tr12);
            RE(c3) = RE(cc[ac-1]) + MUL_R_C(RE(t2),tr12) + MUL_R_C(RE(t3),tr11);
            IM(c3) = IM(cc[ac-1]) + MUL_R_C(IM(t2),tr12) + MUL_R_C(IM(t3),tr11);
            RE(c4) = (MUL_R_C(RE(t5),ti12)*isign - MUL_R_C(RE(t4),ti11));
            IM(c4) = (MUL_R_C(IM(t5),ti12)*isign - MUL_R_C(IM(t4),ti11));
            RE(c5) = (MUL_R_C(RE(t5),ti11)*isign + MUL_R_C(RE(t4),ti12));
            IM(c5) = (MUL_R_C(IM(t5),ti11)*isign + MUL_R_C(IM(t4),ti12));

            RE(ch[ah+l1]) = RE(c2) - IM(c5);
            IM(ch[ah+l1]) = IM(c2) + RE(c5);
            RE(ch[ah+2*l1]) = RE(c3) - IM(c4);
            IM(ch[ah+2*l1]) = IM(c3) + RE(c4);
            RE(ch[ah+3*l1]) = RE(c3) + IM(c4);
            IM(ch[ah+3*l1]) = IM(c3) - RE(c4);
            RE(ch[ah+4*l1]) = RE(c2) + IM(c5);
            IM(ch[ah+4*l1]) = IM(c2) - RE(c5);
        }
    } else {
        for (k = 0; k < l1; k++)
        {
            for (i = 0; i < ido; i++)
            {
                ac = i + (k*5 + 1) * ido;
                ah = i + k * ido;

                RE(t2) = RE(cc[ac]) + RE(cc[ac+3*ido]);
                IM(t2) = IM(cc[ac]) + IM(cc[ac+3*ido]);
                RE(t3) = RE(cc[ac+ido]) + RE(cc[ac+2*ido]);
                IM(t3) = IM(cc[ac+ido]) + IM(cc[ac+2*ido]);
                RE(t4) = RE(cc[ac+ido]) - RE(cc[ac+2*ido]);
                IM(t4) = IM(cc[ac+ido]) - IM(cc[ac+2*ido]);
                RE(t5) = RE(cc[ac]) - RE(cc[ac+3*ido]);
                IM(t5) = IM(cc[ac]) - IM(cc[ac+3*ido]);

                RE(ch[ah]) = RE(cc[ac-ido]) + RE(t2) + RE(t3);
                IM(ch[ah]) = IM(cc[ac-ido]) + IM(t2) + IM(t3);

                RE(c2) = RE(cc[ac-ido]) + MUL_R_C(RE(t2),tr11) + MUL_R_C(RE(t3),tr12);
                IM(c2) = IM(cc[ac-ido]) + MUL_R_C(IM(t2),tr11) + MUL_R_C(IM(t3),tr12);
                RE(c3) = RE(cc[ac-ido]) + MUL_R_C(RE(t2),tr12) + MUL_R_C(RE(t3),tr11);
                IM(c3) = IM(cc[ac-ido]) + MUL_R_C(IM(t2),tr12) + MUL_R_C(IM(t3),tr11);
                RE(c4) = (MUL_R_C(RE(t5),ti12)*isign - MUL_R_C(RE(t4),ti11));
                IM(c4) = (MUL_R_C(IM(t5),ti12)*isign - MUL_R_C(IM(t4),ti11));
                RE(c5) = (MUL_R_C(RE(t5),ti11)*isign + MUL_R_C(RE(t4),ti12));
                IM(c5) = (MUL_R_C(IM(t5),ti11)*isign + MUL_R_C(IM(t4),ti12));

                IM(d2) = IM(c2) + RE(c5);
                IM(d3) = IM(c3) + RE(c4);
                RE(d4) = RE(c3) + IM(c4);
                RE(d5) = RE(c2) + IM(c5);
                RE(d2) = RE(c2) - IM(c5);
                IM(d5) = IM(c2) - RE(c5);
                RE(d3) = RE(c3) - IM(c4);
                IM(d4) = IM(c3) - RE(c4);

                RE(ch[ah+l1*ido]) = MUL_R_C(RE(d2),RE(wa1[i])) - MUL_R_C(IM(d2),IM(wa1[i]))*isign;
                IM(ch[ah+l1*ido]) = MUL_R_C(IM(d2),RE(wa1[i])) + MUL_R_C(RE(d2),IM(wa1[i]))*isign;
                RE(ch[ah+2*l1*ido]) = MUL_R_C(RE(d3),RE(wa2[i])) - MUL_R_C(IM(d3),IM(wa2[i]))*isign;
                IM(ch[ah+2*l1*ido]) = MUL_R_C(IM(d3),RE(wa2[i])) + MUL_R_C(RE(d3),IM(wa2[i]))*isign;
                RE(ch[ah+3*l1*ido]) = MUL_R_C(RE(d4),RE(wa3[i])) - MUL_R_C(IM(d4),IM(wa3[i]))*isign;
                IM(ch[ah+3*l1*ido]) = MUL_R_C(IM(d4),RE(wa3[i])) + MUL_R_C(RE(d4),IM(wa3[i]))*isign;
                RE(ch[ah+4*l1*ido]) = MUL_R_C(RE(d5),RE(wa4[i])) - MUL_R_C(IM(d5),IM(wa4[i]))*isign;
                IM(ch[ah+4*l1*ido]) = MUL_R_C(IM(d5),RE(wa4[i])) + MUL_R_C(RE(d5),IM(wa4[i]))*isign;
            }
        }
    }
}


/*----------------------------------------------------------------------
   cfftf1, cfftf, cfftb, cffti1, cffti. Complex FFTs.
  ----------------------------------------------------------------------*/

INLINE void cfftf1(uint16_t n, complex_t *c, complex_t *ch,
                   uint16_t *ifac, complex_t *wa, int8_t isign)
{
    uint16_t i;
    uint16_t k1, l1, l2;
    uint16_t na, nf, ip, iw, ix2, ix3, ix4, ido, idl1;

    nf = ifac[1];
    na = 0;
    l1 = 1;
    iw = 0;

    for (k1 = 2; k1 <= nf+1; k1++)
    {
        ip = ifac[k1];
        l2 = ip*l1;
        ido = n / l2;
        idl1 = ido*l1;

        switch (ip)
        {
        case 4:
            ix2 = iw + ido;
            ix3 = ix2 + ido;

            if (na == 0)
                passf4(ido, l1, c, ch, &wa[iw], &wa[ix2], &wa[ix3], isign);
            else
                passf4(ido, l1, ch, c, &wa[iw], &wa[ix2], &wa[ix3], isign);

            na = 1 - na;
            break;
        case 2:
            if (na == 0)
                passf2(ido, l1, c, ch, &wa[iw], isign);
            else
                passf2(ido, l1, ch, c, &wa[iw], isign);

            na = 1 - na;
            break;
        case 3:
            ix2 = iw + ido;

            if (na == 0)
                passf3(ido, l1, c, ch, &wa[iw], &wa[ix2], isign);
            else
                passf3(ido, l1, ch, c, &wa[iw], &wa[ix2], isign);

            na = 1 - na;
            break;
        case 5:
            ix2 = iw + ido;
            ix3 = ix2 + ido;
            ix4 = ix3 + ido;

            if (na == 0)
                passf5(ido, l1, c, ch, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);
            else
                passf5(ido, l1, ch, c, &wa[iw], &wa[ix2], &wa[ix3], &wa[ix4], isign);

            na = 1 - na;
            break;
        }

        l1 = l2;
        iw += (ip-1) * ido;
    }

    if (na == 0)
        return;

    for (i = 0; i < n; i++)
    {
        RE(c[i]) = RE(ch[i]);
        IM(c[i]) = IM(ch[i]);
    }
}

void cfftf(cfft_info *cfft, complex_t *c)
{
    cfftf1(cfft->n, c, cfft->work, cfft->ifac, cfft->tab, -1);
}

void cfftb(cfft_info *cfft, complex_t *c)
{
    cfftf1(cfft->n, c, cfft->work, cfft->ifac, cfft->tab, +1);
}

static void cffti1(uint16_t n, complex_t *wa, uint16_t *ifac)
{
    static uint16_t ntryh[4] = {3, 4, 2, 5};
#ifndef FIXED_POINT
    real_t arg, argh, argld, fi;
    uint16_t ido, ipm;
    uint16_t i1, k1, l1, l2;
    uint16_t ld, ii, ip;
#endif
    uint16_t ntry, i, j;
    uint16_t ib;
    uint16_t nf, nl, nq, nr;

    nl = n;
    nf = 0;
    j = 0;

startloop:
    j++;

    if (j <= 4)
        ntry = ntryh[j-1];
    else
        ntry += 2;

    do
    {
        nq = nl / ntry;
        nr = nl - ntry*nq;

        if (nr != 0)
            goto startloop;

        nf++;
        ifac[nf+1] = ntry;
        nl = nq;

        if (ntry == 2 && nf != 1)
        {
            for (i = 2; i <= nf; i++)
            {
                ib = nf - i + 2;
                ifac[ib+1] = ifac[ib];
            }
            ifac[2] = 2;
        }
    } while (nl != 1);

    ifac[0] = n;
    ifac[1] = nf;

#ifndef FIXED_POINT
    argh = (real_t)2.0*M_PI / (real_t)n;
    i = 0;
    l1 = 1;

    for (k1 = 1; k1 <= nf; k1++)
    {
        ip = ifac[k1+1];
        ld = 0;
        l2 = l1*ip;
        ido = n / l2;
        ipm = ip - 1;

        for (j = 0; j < ipm; j++)
        {
            i1 = i;
            RE(wa[i]) = 1.0;
            IM(wa[i]) = 0.0;
            ld += l1;
            fi = 0;
            argld = ld*argh;

            for (ii = 0; ii < ido; ii++)
            {
                i++;
                fi++;
                arg = fi * argld;
                RE(wa[i]) = (real_t)cos(arg);
                IM(wa[i]) = (real_t)sin(arg);
            }

            if (ip > 5)
            {
                RE(wa[i1]) = RE(wa[i]);
                IM(wa[i1]) = IM(wa[i]);
            }
        }
        l1 = l2;
    }
#endif
}

cfft_info *cffti(uint16_t n)
{
    cfft_info *cfft = (cfft_info*)malloc(sizeof(cfft_info));

    cfft->n = n;
    cfft->work = (complex_t*)malloc(n*sizeof(complex_t));

#ifndef FIXED_POINT
    cfft->tab = (complex_t*)malloc(n*sizeof(complex_t));

    cffti1(n, cfft->tab, cfft->ifac);
#else
    cffti1(n, NULL, cfft->ifac);

    switch (n)
    {
    case 64: cfft->tab = cfft_tab_64; break;
    case 512: cfft->tab = cfft_tab_512; break;
#ifdef LD_DEC
    case 256: cfft->tab = cfft_tab_256; break;
#endif

#ifdef ALLOW_SMALL_FRAMELENGTH
    case 60: cfft->tab = cfft_tab_60; break;
    case 480: cfft->tab = cfft_tab_480; break;
#ifdef LD_DEC
    case 240: cfft->tab = cfft_tab_240; break;
#endif
#endif
    }
#endif

    return cfft;
}

void cfftu(cfft_info *cfft)
{
    if (cfft->work) free(cfft->work);
#ifndef FIXED_POINT
    if (cfft->tab) free(cfft->tab);
#endif

    if (cfft) free(cfft);
}