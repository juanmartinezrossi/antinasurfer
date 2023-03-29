#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "permute.h"
#include "turbodecoder.h"

// arrays per decoder
Tdec alfa[NUMSTATES], beta[(SW + GW + 1) * NUMSTATES];

//Tdec LLR1[MAX_LONG_CB + TOTALTAIL], LLR2[MAX_LONG_CB + TOTALTAIL], W[MAX_LONG_CB + TOTALTAIL];
Tdec LLR1[MAX_LONG_CODED ], LLR2[MAX_LONG_CODED ], W[MAX_LONG_CODED ];

Tdec data[RATE*(SW + GW + TOTALTAIL)];
Tdec parity[RATE*(SW + GW + TOTALTAIL)];

struct permute_t permuta;

int iteration;
int HALT_min;

int start, end;

void precach_full(Tdec *input, Tper *per, int dec) {
    int i;

    /* copy data temporal buffer (to allow fastest loops)*/
    memcpy(data, input, RATE * (SW + GW) * sizeof (Tdec));
    memcpy(parity, &input[dec], RATE * (SW + GW) * sizeof (Tdec));

    if (dec == 1) {
        for (i = 0; i < SW + GW; i++) {
            data[RATE * i] += W[i + start];
        }
    } else {
        for (i = 0; i < SW + GW; i++) {
            data[RATE * i] = LLR1[per[i + start]] - W[per[i + start]];
        }
    }
}

void precach_end(Tdec *input, Tper *per, int end, int dec) {
    int i;

    /* copy data temporal buffer (to allow fastest loops)*/
    memcpy(data, input, RATE * (SW + GW) * sizeof (Tdec));
    memcpy(parity, &input[dec], RATE * (SW + GW) * sizeof (Tdec));

    if (dec == 1) {
        for (i = 0; i < end; i++) {
            data[RATE * i] += W[i + start];
        }
    } else {
        for (i = 0; i < end; i++) {
            data[RATE * i] = LLR1[per[i + start]] - W[per[i + start]];
        }
    }
}

void compute_beta_full() {
    Tdec m_b0, m_b1, m_b2, m_b3, m_b4, m_b5, m_b6, m_b7;
    Tdec new0, new1, new2, new3, new4, new5, new6, new7;
    Tdec old0, old1, old2, old3, old4, old5, old6, old7;

    Tdec x, y, xy;
    int k;

    old0 = beta[8 * (SW + GW) + 0];
    old1 = beta[8 * (SW + GW) + 1];
    old2 = beta[8 * (SW + GW) + 2];
    old3 = beta[8 * (SW + GW) + 3];
    old4 = beta[8 * (SW + GW) + 4];
    old5 = beta[8 * (SW + GW) + 5];
    old6 = beta[8 * (SW + GW) + 6];
    old7 = beta[8 * (SW + GW) + 7];

    for (k = SW + GW - 1; k >= 0; k--) {
        x = data[RATE * k];
        y = parity[RATE * k];
        xy = x + y;

        m_b0 = old4 + xy;
        m_b1 = old4;
        m_b2 = old5 + y;
        m_b3 = old5 + x;
        m_b4 = old6 + x;
        m_b5 = old6 + y;
        m_b6 = old7;
        m_b7 = old7 + xy;

        new0 = old0;
        new1 = old0 + xy;
        new2 = old1 + x;
        new3 = old1 + y;
        new4 = old2 + y;
        new5 = old2 + x;
        new6 = old3 + xy;
        new7 = old3;

        if (m_b0 > new0) new0 = m_b0;
        beta[8 * k + 0] = new0;
        old0 = new0;

        if (m_b1 > new1) new1 = m_b1;
        beta[8 * k + 1] = new1;
        old1 = new1;

        if (m_b2 > new2) new2 = m_b2;
        beta[8 * k + 2] = new2;
        old2 = new2;

        if (m_b3 > new3) new3 = m_b3;
        beta[8 * k + 3] = new3;
        old3 = new3;

        if (m_b4 > new4) new4 = m_b4;
        beta[8 * k + 4] = new4;
        old4 = new4;

        if (m_b5 > new5) new5 = m_b5;
        beta[8 * k + 5] = new5;
        old5 = new5;

        if (m_b6 > new6) new6 = m_b6;
        beta[8 * k + 6] = new6;
        old6 = new6;

        if (m_b7 > new7) new7 = m_b7;
        beta[8 * k + 7] = new7;
        old7 = new7;

    }
}

void compute_beta_end(int end, int dec, int long_cb) {
    Tdec m_b0, m_b1, m_b2, m_b3, m_b4, m_b5, m_b6, m_b7;
    Tdec new0, new1, new2, new3, new4, new5, new6, new7;
    Tdec old0, old1, old2, old3, old4, old5, old6, old7;

    Tdec x, y, xy;
    int k;

    old0 = beta[8 * (end) + 0];
    old1 = beta[8 * (end) + 1];
    old2 = beta[8 * (end) + 2];
    old3 = beta[8 * (end) + 3];
    old4 = beta[8 * (end) + 4];
    old5 = beta[8 * (end) + 5];
    old6 = beta[8 * (end) + 6];
    old7 = beta[8 * (end) + 7];

    for (k = end - 1; k >= 0; k--) {
        if (k + start > long_cb - 1) {
            if (dec == 1) {
                x = data[RATE * (long_cb - start) + NINPUTS * (k + start - long_cb)];
                y = data[RATE * (long_cb - start) + NINPUTS * (k + start - long_cb) + 1];
            } else {
                x = data[RATE * (long_cb - start) + NINPUTS * RATE + NINPUTS * (k + start - long_cb)];
                y = data[RATE * (long_cb - start) + NINPUTS * RATE + NINPUTS * (k + start - long_cb) + 1];
            }
        } else {
            x = data[RATE * k];
            y = parity[RATE * k];
        }
        xy = x + y;

        m_b0 = old4 + xy;
        m_b1 = old4;
        m_b2 = old5 + y;
        m_b3 = old5 + x;
        m_b4 = old6 + x;
        m_b5 = old6 + y;
        m_b6 = old7;
        m_b7 = old7 + xy;

        new0 = old0;
        new1 = old0 + xy;
        new2 = old1 + x;
        new3 = old1 + y;
        new4 = old2 + y;
        new5 = old2 + x;
        new6 = old3 + xy;
        new7 = old3;

        if (m_b0 > new0) new0 = m_b0;
        beta[8 * k + 0] = new0;
        old0 = new0;

        if (m_b1 > new1) new1 = m_b1;
        beta[8 * k + 1] = new1;
        old1 = new1;

        if (m_b2 > new2) new2 = m_b2;
        beta[8 * k + 2] = new2;
        old2 = new2;

        if (m_b3 > new3) new3 = m_b3;
        beta[8 * k + 3] = new3;
        old3 = new3;

        if (m_b4 > new4) new4 = m_b4;
        beta[8 * k + 4] = new4;
        old4 = new4;

        if (m_b5 > new5) new5 = m_b5;
        beta[8 * k + 5] = new5;
        old5 = new5;

        if (m_b6 > new6) new6 = m_b6;
        beta[8 * k + 6] = new6;
        old6 = new6;

        if (m_b7 > new7) new7 = m_b7;
        beta[8 * k + 7] = new7;
        old7 = new7;

    }
}

void compute_alfa_full(int dec, Tdec *output) {
    Tdec m_b0, m_b1, m_b2, m_b3, m_b4, m_b5, m_b6, m_b7;
    Tdec new0, new1, new2, new3, new4, new5, new6, new7;
    Tdec old0, old1, old2, old3, old4, old5, old6, old7;
    Tdec max1_0, max1_1, max1_2, max1_3, max1_4, max1_5, max1_6, max1_7;
    Tdec max0_0, max0_1, max0_2, max0_3, max0_4, max0_5, max0_6, max0_7;
    Tdec m1, m0;
    Tdec x, y, xy;
    Tdec out;
    int k;


    old0 = alfa[0];
    old1 = alfa[1];
    old2 = alfa[2];
    old3 = alfa[3];
    old4 = alfa[4];
    old5 = alfa[5];
    old6 = alfa[6];
    old7 = alfa[7];

    for (k = 1; k < SW + 1; k++) {
        x = data[RATE * (k - 1)];
        y = parity[RATE * (k - 1)];

        xy = x + y;

        m_b0 = old0;
        m_b1 = old3 + y;
        m_b2 = old4 + y;
        m_b3 = old7;
        m_b4 = old1;
        m_b5 = old2 + y;
        m_b6 = old5 + y;
        m_b7 = old6;

        new0 = old1 + xy;
        new1 = old2 + x;
        new2 = old5 + x;
        new3 = old6 + xy;
        new4 = old0 + xy;
        new5 = old3 + x;
        new6 = old4 + x;
        new7 = old7 + xy;

        max0_0 = m_b0 + beta[8 * k + 0];
        max0_1 = m_b1 + beta[8 * k + 1];
        max0_2 = m_b2 + beta[8 * k + 2];
        max0_3 = m_b3 + beta[8 * k + 3];
        max0_4 = m_b4 + beta[8 * k + 4];
        max0_5 = m_b5 + beta[8 * k + 5];
        max0_6 = m_b6 + beta[8 * k + 6];
        max0_7 = m_b7 + beta[8 * k + 7];

        max1_0 = new0 + beta[8 * k + 0];
        max1_1 = new1 + beta[8 * k + 1];
        max1_2 = new2 + beta[8 * k + 2];
        max1_3 = new3 + beta[8 * k + 3];
        max1_4 = new4 + beta[8 * k + 4];
        max1_5 = new5 + beta[8 * k + 5];
        max1_6 = new6 + beta[8 * k + 6];
        max1_7 = new7 + beta[8 * k + 7];

        m1 = max1_0;
        if (max1_1 > m1) m1 = max1_1;
        if (max1_2 > m1) m1 = max1_2;
        if (max1_3 > m1) m1 = max1_3;
        if (max1_4 > m1) m1 = max1_4;
        if (max1_5 > m1) m1 = max1_5;
        if (max1_6 > m1) m1 = max1_6;
        if (max1_7 > m1) m1 = max1_7;

        m0 = max0_0;
        if (max0_1 > m0) m0 = max0_1;
        if (max0_2 > m0) m0 = max0_2;
        if (max0_3 > m0) m0 = max0_3;
        if (max0_4 > m0) m0 = max0_4;
        if (max0_5 > m0) m0 = max0_5;
        if (max0_6 > m0) m0 = max0_6;
        if (max0_7 > m0) m0 = max0_7;


        if (m_b0 > new0) new0 = m_b0;
        old0 = new0;

        if (m_b1 > new1) new1 = m_b1;
        old1 = new1;

        if (m_b2 > new2) new2 = m_b2;
        old2 = new2;

        if (m_b3 > new3) new3 = m_b3;
        old3 = new3;

        if (m_b4 > new4) new4 = m_b4;
        old4 = new4;

        if (m_b5 > new5) new5 = m_b5;
        old5 = new5;

        if (m_b6 > new6) new6 = m_b6;
        old6 = new6;

        if (m_b7 > new7) new7 = m_b7;
        old7 = new7;

        out = m1 - m0;

        if (dec == 2) {
            if (abs(out) < HALT_min) {
                HALT_min = abs(out);
            }
        }
        output[k - 1] = out;
    }

    alfa[0] = old0;
    alfa[1] = old1;
    alfa[2] = old2;
    alfa[3] = old3;
    alfa[4] = old4;
    alfa[5] = old5;
    alfa[6] = old6;
    alfa[7] = old7;
}

void compute_alfa_end(int end, int dec, Tdec *output) {
    Tdec m_b0, m_b1, m_b2, m_b3, m_b4, m_b5, m_b6, m_b7;
    Tdec new0, new1, new2, new3, new4, new5, new6, new7;
    Tdec old0, old1, old2, old3, old4, old5, old6, old7;
    Tdec max1_0, max1_1, max1_2, max1_3, max1_4, max1_5, max1_6, max1_7;
    Tdec max0_0, max0_1, max0_2, max0_3, max0_4, max0_5, max0_6, max0_7;
    Tdec m1, m0;
    Tdec x, y, xy;
    Tdec out;
    int k;


    old0 = alfa[0];
    old1 = alfa[1];
    old2 = alfa[2];
    old3 = alfa[3];
    old4 = alfa[4];
    old5 = alfa[5];
    old6 = alfa[6];
    old7 = alfa[7];

    for (k = 1; k < end + 1; k++) {
        x = data[RATE * (k - 1)];
        y = parity[RATE * (k - 1)];

        xy = x + y;

        m_b0 = old0;
        m_b1 = old3 + y;
        m_b2 = old4 + y;
        m_b3 = old7;
        m_b4 = old1;
        m_b5 = old2 + y;
        m_b6 = old5 + y;
        m_b7 = old6;

        new0 = old1 + xy;
        new1 = old2 + x;
        new2 = old5 + x;
        new3 = old6 + xy;
        new4 = old0 + xy;
        new5 = old3 + x;
        new6 = old4 + x;
        new7 = old7 + xy;

        max0_0 = m_b0 + beta[8 * k + 0];
        max0_1 = m_b1 + beta[8 * k + 1];
        max0_2 = m_b2 + beta[8 * k + 2];
        max0_3 = m_b3 + beta[8 * k + 3];
        max0_4 = m_b4 + beta[8 * k + 4];
        max0_5 = m_b5 + beta[8 * k + 5];
        max0_6 = m_b6 + beta[8 * k + 6];
        max0_7 = m_b7 + beta[8 * k + 7];

        max1_0 = new0 + beta[8 * k + 0];
        max1_1 = new1 + beta[8 * k + 1];
        max1_2 = new2 + beta[8 * k + 2];
        max1_3 = new3 + beta[8 * k + 3];
        max1_4 = new4 + beta[8 * k + 4];
        max1_5 = new5 + beta[8 * k + 5];
        max1_6 = new6 + beta[8 * k + 6];
        max1_7 = new7 + beta[8 * k + 7];

        m1 = max1_0;
        if (max1_1 > m1) m1 = max1_1;
        if (max1_2 > m1) m1 = max1_2;
        if (max1_3 > m1) m1 = max1_3;
        if (max1_4 > m1) m1 = max1_4;
        if (max1_5 > m1) m1 = max1_5;
        if (max1_6 > m1) m1 = max1_6;
        if (max1_7 > m1) m1 = max1_7;

        m0 = max0_0;
        if (max0_1 > m0) m0 = max0_1;
        if (max0_2 > m0) m0 = max0_2;
        if (max0_3 > m0) m0 = max0_3;
        if (max0_4 > m0) m0 = max0_4;
        if (max0_5 > m0) m0 = max0_5;
        if (max0_6 > m0) m0 = max0_6;
        if (max0_7 > m0) m0 = max0_7;


        if (m_b0 > new0) new0 = m_b0;
        old0 = new0;

        if (m_b1 > new1) new1 = m_b1;
        old1 = new1;

        if (m_b2 > new2) new2 = m_b2;
        old2 = new2;

        if (m_b3 > new3) new3 = m_b3;
        old3 = new3;

        if (m_b4 > new4) new4 = m_b4;
        old4 = new4;

        if (m_b5 > new5) new5 = m_b5;
        old5 = new5;

        if (m_b6 > new6) new6 = m_b6;
        old6 = new6;

        if (m_b7 > new7) new7 = m_b7;
        old7 = new7;

        out = m1 - m0;

        if (dec == 2) {
            if (abs(out) < HALT_min) {
                HALT_min = abs(out);
            }
        }
        output[k - 1] = out;
    }

    alfa[0] = old0;
    alfa[1] = old1;
    alfa[2] = old2;
    alfa[3] = old3;
    alfa[4] = old4;
    alfa[5] = old5;
    alfa[6] = old6;
    alfa[7] = old7;
}

void DEC_RSC(Tdec *input, Tdec *output, Tper *per, ctrl_turbodecoder_t *control, int dec) {
    int k;
    int long_cb;
    int fullend;

    start = 0;
    end = SW + GW;
    if (end > control->Long_CodeBlock) end = control->Long_CodeBlock + TAIL + 1;

    long_cb = control->Long_CodeBlock;

    /** Initialize alfa states */
    alfa[0] = 0;
    for (k = 1; k < NUMSTATES; k++) {
        alfa[k] = -INF;
    }

    while (start < long_cb + TAIL + 1) {

        // inicialitzem beta
        fullend = SW + GW;

        if (fullend + start > long_cb + TAIL + 1) {
            fullend = long_cb + TAIL - start;

            beta[fullend * NUMSTATES] = 0;
            for (k = 1; k < NUMSTATES; k++)
                beta[fullend * NUMSTATES + k] = -INF;

            precach_end(&input[RATE * start], per, long_cb - start, dec);

            compute_beta_end(fullend, dec, long_cb);

            // calculem alfa i llr
            compute_alfa_end(fullend - TAIL, dec, &output[start]);

        } else {
            for (k = 0; k < NUMSTATES; k++)
                beta[(SW + GW) * NUMSTATES + k] = (Tdec) (LOG18 * ESCALA);

            precach_full(&input[RATE * start], per, dec);

            compute_beta_full();

            // calculem alfa i llr
            compute_alfa_full(dec, &output[start]);

        }

        if ((end - long_cb) < GW && end > long_cb)
            start = end;
        else
            start = end - GW;

        end = end + SW;
        if (end > long_cb + TAIL + 1)
            end = long_cb + TAIL + 1;
    }
}

void decide(char *output, Tper *desper, int long_cb) {
    int i;

    for (i = 0; i < long_cb; i++)
        output[i] = (LLR2[desper[i]] > 0) ? 1 : 0;

}

void update_W(Tper *desper, int long_cb) {
    int i;

    for (i = 0; i < long_cb; i++) {
        W[i] += LLR2[desper[i]] - LLR1[i];
    }
}
// ========================== FUNCIONS DESCODIFICACI�

int TurboDecoder(Tdec *input, char *output, ctrl_turbodecoder_t *codecfg)
{
    int i;
    int * halt = &(codecfg->halt);
    int type = codecfg->type;
    long halt_mean=0;
    int stop=0;
    iteration = 0;

        HALT_min = INF;

    ComputePermutation(&permuta, codecfg->Long_CodeBlock, type);

    memset(W, 0, sizeof (Tdec) * codecfg->Long_CodeBlock);

    do {
        if (iteration)
            update_W(permuta.DESPER, codecfg->Long_CodeBlock);

        // 1er DECODER
        DEC_RSC(input, LLR1, permuta.PER, codecfg, 1);

        HALT_min = INF;

        // 2on DECODER
        DEC_RSC(input, LLR2, permuta.PER, codecfg, 2);

        iteration++;

        switch(codecfg->haltMethod) {
        case HALT_METHOD_MEAN:
            for (i=0;i<codecfg->Long_CodeBlock;i++) {
                halt_mean+=(long) abs(LLR2[i]);
            }
            halt_mean/=codecfg->Long_CodeBlock;
            if (halt_mean >  codecfg->Turbo_Dt)
                stop=1;
            if (halt != NULL) {
                *halt = halt_mean;
            }
        break;
        case HALT_METHOD_MIN:
            if (HALT_min>codecfg->Turbo_Dt) {
        	stop=1;
            }
            if (halt != NULL) {
                *halt = HALT_min;
            }
        break;
        default:
        	for (i=0;i<codecfg->Long_CodeBlock;i++) {
				halt_mean+=(long) abs(LLR2[i]);
			}
        	halt_mean/=codecfg->Long_CodeBlock;
        	stop=0;
        	if (halt != NULL) {
				*halt = halt_mean;
			}
        }


    } while (iteration < codecfg->Turbo_iteracions && stop==0);
    decide(output, permuta.DESPER, codecfg->Long_CodeBlock);

    return iteration;
}

