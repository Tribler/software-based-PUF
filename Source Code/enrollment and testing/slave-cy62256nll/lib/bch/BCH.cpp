// ------------------------------------------------------------------------
//
//        File: BCH.cpp
//        Date: April 15, 2018
//        Created by: Ade Setyawan Sajim as part of his MSc Thesis project
//
// This code is a modified version from bch_erasures.c created by R.H. Morelos
// Zaragoza which can be retrieved at
// http://www.the-art-of-ecc.com/3_Cyclic_BCH/bch_erasures.c.
// Description: Encoding and decoding routines for binary BCH codes
//              Erasure correction by two errors-only decoding passes using
//              the EUCLIDEAN ALGORITHM
//

#include "BCH.h"

BCH::BCH() {

}


void BCH::initialize() {
    memset(p, 0, sizeof(p));
    memset(alpha_to, 0, sizeof(alpha_to));
    memset(index_of, 0, sizeof(index_of));
    memset(g, 0, sizeof(g));

    tools = Tools();
    initialize_p();
    generate_gf();
    gen_poly();
}

void
BCH::initialize_p()
/*
 *	Read m, the degree of a primitive polynomial p(x) used to compute the
 *	Galois field GF(2**m). Get precomputed coefficients p[] of p(x). Read
 *	the code length.
 */
{
    int i;
    for (i = 1; i < m; i++)
        p[i] = 0;
    p[0] = p[m] = 1;
    if (m == 2) p[1] = 1;
    else if (m == 3) p[1] = 1;
    else if (m == 4) p[1] = 1;
    else if (m == 5) p[2] = 1;
    else if (m == 6) p[1] = 1;
    else if (m == 7) p[1] = 1;
    else if (m == 8) p[4] = p[5] = p[6] = 1;
    else if (m == 9) p[4] = 1;
    else if (m == 10) p[3] = 1;
    else if (m == 11) p[2] = 1;
    else if (m == 12) p[3] = p[4] = p[7] = 1;
    else if (m == 13) p[1] = p[3] = p[4] = 1;
    else if (m == 14) p[1] = p[11] = p[12] = 1;
    else if (m == 15) p[1] = 1;
    else if (m == 16) p[2] = p[3] = p[5] = 1;
    else if (m == 17) p[3] = 1;
    else if (m == 18) p[7] = 1;
    else if (m == 19) p[1] = p[5] = p[6] = 1;
    else if (m == 20) p[3] = 1;
}


void
BCH::generate_gf()
/*
 * Generate field GF(2**m) from the irreducible polynomial p(X) with
 * coefficients in p[0]..p[m].
 *
 * Lookup tables:
 *   index->polynomial form: alpha_to[] contains j=alpha^i;
 *   polynomial form -> index form:	index_of[j=alpha^i] = i
 *
 * alpha=2 is the primitive element of GF(2**m)
 */
{
    register int i, mask;

    mask = 1;
    alpha_to[m] = 0;
    for (i = 0; i < m; i++) {
        alpha_to[i] = mask;
        index_of[alpha_to[i]] = i;
        if (p[i] != 0)
            alpha_to[m] ^= mask;
        mask <<= 1;
    }
    index_of[alpha_to[m]] = m;
    mask >>= 1;
    for (i = m + 1; i < n; i++) {
        if (alpha_to[i - 1] >= mask)
            alpha_to[i] = alpha_to[m] ^ ((alpha_to[i - 1] ^ mask) << 1);
        else
            alpha_to[i] = alpha_to[i - 1] << 1;
        index_of[alpha_to[i]] = i;
    }
    index_of[0] = -1;
}


void
BCH::gen_poly()
/*
 * Compute the generator polynomial of a binary BCH code. Fist generate the
 * cycle sets modulo 2**m - 1, cycle[][] =  (i, 2*i, 4*i, ..., 2^l*i). Then
 * determine those cycle sets that contain integers in the set of (d-1)
 * consecutive integers {1..(d-1)}. The generator polynomial is calculated
 * as the product of linear factors of the form (x+alpha^i), for every i in
 * the above cycle sets.
 */
{
    register int8_t ii, jj, ll, kaux;
    register int8_t test, aux, nocycles, root, noterms, rdncy;
    int8_t cycle[(int) pow(2, m)][m], size[(int8_t) pow(2, m)], min[(int8_t) pow(2, m)], zeros[(int8_t) pow(2, m)];

    memset(cycle, 0, sizeof(cycle));
    memset(size, 0, sizeof(size));
    memset(min, 0, sizeof(min));
    memset(zeros, 0, sizeof(zeros));

    /* Generate cycle sets modulo n, n = 2**m - 1 */
    cycle[0][0] = 0;
    size[0] = 1;
    cycle[1][0] = 1;
    size[1] = 1;
    jj = 1;            /* cycle set index */
    do {
        /* Generate the jj-th cycle set */
        ii = 0;
        do {
            ii++;
            cycle[jj][ii] = (cycle[jj][ii - 1] * 2) % n;
            size[jj]++;
            aux = (cycle[jj][ii] * 2) % n;
        } while (aux != cycle[jj][0]);
        /* Next cycle set representative */
        ll = 0;
        do {
            ll++;
            test = 0;
            for (ii = 1; ((ii <= jj) && (!test)); ii++)
                /* Examine previous cycle sets */
                for (kaux = 0; ((kaux < size[ii]) && (!test)); kaux++)
                    if (ll == cycle[ii][kaux])
                        test = 1;
        } while ((test) && (ll < (n - 1)));
        if (!(test)) {
            jj++;    /* next cycle set index */
            cycle[jj][0] = ll;
            size[jj] = 1;
        }
    } while (ll < (n - 1));
    nocycles = jj;        /* number of cycle sets modulo n */

    d = 2 * t + 1;

    /* Search for roots 1, 2, ..., d-1 in cycle sets */
    kaux = 0;
    rdncy = 0;
    for (ii = 1; ii <= nocycles; ii++) {
        min[kaux] = 0;
        test = 0;
        for (jj = 0; ((jj < size[ii]) && (!test)); jj++)
            for (root = 1; ((root < d) && (!test)); root++)
                if (root == cycle[ii][jj]) {
                    test = 1;
                    min[kaux] = ii;
                }
        if (min[kaux]) {
            rdncy += size[min[kaux]];
            kaux++;
        }
    }
    noterms = kaux;
    kaux = 1;
    for (ii = 0; ii < noterms; ii++)
        for (jj = 0; jj < size[min[ii]]; jj++) {
            zeros[kaux] = cycle[min[ii]][jj];
            kaux++;
        }

    k = n - rdncy;

    if (k < 0) {
        Serial.println("Parameters invalid!\n");
        exit(0);
    }

    /* Compute the generator polynomial */
    g[0] = alpha_to[zeros[1]];
    g[1] = 1;        /* g(x) = (X + zeros[1]) initially */
    for (ii = 2; ii <= rdncy; ii++) {
        g[ii] = 1;
        for (jj = ii - 1; jj > 0; jj--)
            if (g[jj] != 0)
                g[jj] = g[jj - 1] ^ alpha_to[(index_of[g[jj]] + zeros[ii]) % n];
            else
                g[jj] = g[jj - 1];
        g[0] = alpha_to[(index_of[g[0]] + zeros[ii]) % n];
    }
}


void BCH::encode_bch(uint8_t *in, uint8_t *result)
/*
 * Compute redundacy bb[], the coefficients of b(x). The redundancy
 * polynomial b(x) is the remainder after dividing x^(length-k)*data(x)
 * by the generator polynomial g(x).
 */
{
    register int8_t i, j;
    register int8_t feedback;

    int8_t input[8];
    memset(input, 0, sizeof(input));
    tools.convert_uint8_array_to_bin_array(in, input, 1);

    int8_t bb[n - k];
    for (i = 0; i < n - k; i++)
        bb[i] = 0;
    for (i = k - 1; i >= 0; i--) {
        feedback = input[i] ^ bb[n - k - 1];
        if (feedback != 0) {
            for (j = n - k - 1; j > 0; j--)
                if (g[j] != 0)
                    bb[j] = bb[j - 1] ^ feedback;
                else
                    bb[j] = bb[j - 1];
            bb[0] = g[0] && feedback;
        } else {
            for (j = n - k - 1; j > 0; j--)
                bb[j] = bb[j - 1];
            bb[0] = 0;
        }
    }

    tools.combine_two_bin_array_to_uint8_array(bb, n - k, input, k, result);
}

void
BCH::decode_bch(uint8_t *in, uint8_t *result)
/*
 * Simon Rockliff's implementation of Berlekamp's algorithm.
 *
 * Assume we have received bits in recd[i], i=0..(n-1).
 *
 * Compute the 2*t syndromes by substituting alpha^i into rec(X) and
 * evaluating, storing the syndromes in s[i], i=1..2t (leave s[0] zero) .
 * Then we use the Berlekamp algorithm to find the error location polynomial
 * elp[i].
 *
 * If the degree of the elp is >t, then we cannot correct all the errors, and
 * we have detected an uncorrectable error pattern. We output the information
 * bits uncorrected.
 *
 * If the degree of elp is <=t, we substitute alpha^i , i=1..n into the elp
 * to get the roots, hence the inverse roots, the error location numbers.
 * This step is usually called "Chien's search".
 *
 * If the number of errors located is not equal the degree of the elp, then
 * the decoder assumes that there are more than t errors and cannot correct
 * them, only detect them. We output the information bits uncorrected.
 */
{
    int8_t i, j, u, q, t2, count = 0, syn_error = 0;
    int8_t elp[(int) pow(2, m) + 2][(int) pow(2, m)], d[(int) pow(2, m) + 2], l[(int) pow(2, m) + 2], u_lu[
            (int) pow(2, m) + 2], s[(int) pow(2, m) + 1];
    int8_t root[200], loc[200], err[(int) pow(2, m)], reg[201];

    // memset(elp, 0, sizeof(elp));
    // memset(d, 0, sizeof(d));
    // memset(l, 0, sizeof(l));
    // memset(u_lu, 0, sizeof(u_lu));
    // memset(s, 0, sizeof(s));
    // memset(root, 0, sizeof(root));
    // memset(loc, 0, sizeof(loc));
    // memset(err, 0, sizeof(err));
    // memset(reg, 0, sizeof(reg));
    // memset(input, 0, sizeof(input));

    int8_t input[64];
    memset(input, 0, sizeof(input));
    tools.convert_uint8_array_to_bin_array(in, input, 8);

    // decode_bch_old(input, result);
    t2 = 2 * t;

    /* first form the syndromes */
    for (i = 1; i <= t2; i++) {
        s[i] = 0;
        for (j = 0; j < n; j++)
            if (input[j] != 0)
                s[i] ^= alpha_to[(i * j) % n];
        if (s[i] != 0)
            syn_error = 1; /* set error flag if non-zero syndrome */
/*
 * Note:    If the code is used only for ERROR DETECTION, then
 *          exit program here indicating the presence of errors.
 */
        /* convert syndrome from polynomial form to index form  */
        s[i] = index_of[s[i]];
    }

    if (syn_error) {    /* if there are errors, try to correct them */
        /*
         * Compute the error location polynomial via the Berlekamp
         * iterative algorithm. Following the terminology of Lin and
         * Costello's book :   d[u] is the 'mu'th discrepancy, where
         * u='mu'+1 and 'mu' (the Greek letter!) is the step number
         * ranging from -1 to 2*t (see L&C),  l[u] is the degree of
         * the elp at that step, and u_l[u] is the difference between
         * the step number and the degree of the elp.
         */
        /* initialise table entries */
        d[0] = 0;            /* index form */
        d[1] = s[1];        /* index form */
        elp[0][0] = 0;        /* index form */
        elp[1][0] = 1;        /* polynomial form */
        for (i = 1; i < t2; i++) {
            elp[0][i] = -1;    /* index form */
            elp[1][i] = 0;    /* polynomial form */
        }
        l[0] = 0;
        l[1] = 0;
        u_lu[0] = -1;
        u_lu[1] = 0;
        u = 0;

        do {
            u++;
            if (d[u] == -1) {
                l[u + 1] = l[u];
                for (i = 0; i <= l[u]; i++) {
                    elp[u + 1][i] = elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            } else
                /*
                 * search for words with greatest u_lu[q] for
                 * which d[q]!=0
                 */
            {
                q = u - 1;
                while ((d[q] == -1) && (q > 0))
                    q--;
                /* have found first non-zero d[q]  */
                if (q > 0) {
                    j = q;
                    do {
                        j--;
                        if ((d[j] != -1) && (u_lu[q] < u_lu[j]))
                            q = j;
                    } while (j > 0);
                }

                /*
                 * have now found q such that d[u]!=0 and
                 * u_lu[q] is maximum
                 */
                /* store degree of new elp polynomial */
                if (l[u] > l[q] + u - q)
                    l[u + 1] = l[u];
                else
                    l[u + 1] = l[q] + u - q;

                /* form new elp(x) */
                for (i = 0; i < t2; i++)
                    elp[u + 1][i] = 0;
                for (i = 0; i <= l[q]; i++)
                    if (elp[q][i] != -1)
                        elp[u + 1][i + u - q] =
                                alpha_to[(d[u] + n - d[q] + elp[q][i]) % n];
                for (i = 0; i <= l[u]; i++) {
                    elp[u + 1][i] ^= elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            }
            u_lu[u + 1] = u - l[u + 1];

            /* form (u+1)th discrepancy */
            if (u < t2) {
                /* no discrepancy computed on last iteration */
                if (s[u + 1] != -1)
                    d[u + 1] = alpha_to[s[u + 1]];
                else
                    d[u + 1] = 0;
                for (i = 1; i <= l[u + 1]; i++)
                    if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0))
                        d[u + 1] ^= alpha_to[(s[u + 1 - i]
                                              + index_of[elp[u + 1][i]]) % n];
                /* put d[u+1] into index form */
                d[u + 1] = index_of[d[u + 1]];
            }
        } while ((u < t2) && (l[u + 1] <= t));

        u++;
        if (l[u] <= t) {/* Can correct errors */
            /* put elp into index form */
            for (i = 0; i <= l[u]; i++)
                elp[u][i] = index_of[elp[u][i]];

            /* Chien search: find roots of the error location polynomial */
            for (i = 1; i <= l[u]; i++)
                reg[i] = elp[u][i];
            count = 0;
            for (i = 1; i <= n; i++) {
                q = 1;
                for (j = 1; j <= l[u]; j++)
                    if (reg[j] != -1) {
                        reg[j] = (reg[j] + j) % n;
                        q ^= alpha_to[reg[j]];
                    }
                if (!q) {    /* store root and error
						 * location number indices */
                    root[count] = i;
                    loc[count] = n - i;
                    count++;
                }
            }
            if (count == l[u])
                /* no. roots = degree of elp hence <= t errors */
                for (i = 0; i < l[u]; i++)
                    input[loc[i]] ^= 1;
            // else    /* elp has degree >t hence cannot solve */
            //     Serial.println("Incomplete decoding: errors detected\n");
        }
    }

    tools.convert_bin_array_to_uint8_array(&input[n - k], result, k);
//    memcpy(result, &input[n - k], sizeof(int8_t) * k);
}

void BCH::decode_bch_old(int8_t *input, uint8_t *result)
/*
 * Simon Rockliff's implementation of Berlekamp's algorithm.
 *
 * Assume we have received bits in recd[i], i=0..(n-1).
 *
 * Compute the 2*t syndromes by substituting alpha^i into rec(X) and
 * evaluating, storing the syndromes in s[i], i=1..2t (leave s[0] zero) .
 * Then we use the Berlekamp algorithm to find the error location polynomial
 * elp[i].
 *
 * If the degree of the elp is >t, then we cannot correct all the errors, and
 * we have detected an uncorrectable error pattern. We output the information
 * bits uncorrected.
 *
 * If the degree of elp is <=t, we substitute alpha^i , i=1..n into the elp
 * to get the roots, hence the inverse roots, the error location numbers.
 * This step is usually called "Chien's search".
 *
 * If the number of errors located is not equal the degree of the elp, then
 * the decoder assumes that there are more than t errors and cannot correct
 * them, only detect them. We output the information bits uncorrected.
 */
{
    register int8_t i, j, u, q, t2, count = 0, syn_error = 0;
    int8_t elp[(int)pow(2,m)+2][(int)pow(2,m)], d[(int)pow(2,m)+2], l[(int)pow(2,m)+2], u_lu[(int)pow(2,m)+2], s[(int)pow(2,m)+1];
    int8_t root[200], loc[200], err[(int)pow(2,m)], reg[201];

    t2 = 2 * t;

    /* first form the syndromes */
    for (i = 1; i <= t2; i++) {
        s[i] = 0;
        for (j = 0; j < n; j++)
            if (input[j] != 0)
                s[i] ^= alpha_to[(i * j) % n];
        if (s[i] != 0)
            syn_error = 1; /* set error flag if non-zero syndrome */
/*
 * Note:    If the code is used only for ERROR DETECTION, then
 *          exit program here indicating the presence of errors.
 */
        /* convert syndrome from polynomial form to index form  */
        s[i] = index_of[s[i]];
    }

    if (syn_error) {	/* if there are errors, try to correct them */
        /*
         * Compute the error location polynomial via the Berlekamp
         * iterative algorithm. Following the terminology of Lin and
         * Costello's book :   d[u] is the 'mu'th discrepancy, where
         * u='mu'+1 and 'mu' (the Greek letter!) is the step number
         * ranging from -1 to 2*t (see L&C),  l[u] is the degree of
         * the elp at that step, and u_l[u] is the difference between
         * the step number and the degree of the elp.
         */
        /* initialise table entries */
        d[0] = 0;			/* index form */
        d[1] = s[1];		/* index form */
        elp[0][0] = 0;		/* index form */
        elp[1][0] = 1;		/* polynomial form */
        for (i = 1; i < t2; i++) {
            elp[0][i] = -1;	/* index form */
            elp[1][i] = 0;	/* polynomial form */
        }
        l[0] = 0;
        l[1] = 0;
        u_lu[0] = -1;
        u_lu[1] = 0;
        u = 0;

        do {
            u++;
            if (d[u] == -1) {
                l[u + 1] = l[u];
                for (i = 0; i <= l[u]; i++) {
                    elp[u + 1][i] = elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            } else
                /*
                 * search for words with greatest u_lu[q] for
                 * which d[q]!=0
                 */
            {
                q = u - 1;
                while ((d[q] == -1) && (q > 0))
                    q--;
                /* have found first non-zero d[q]  */
                if (q > 0) {
                    j = q;
                    do {
                        j--;
                        if ((d[j] != -1) && (u_lu[q] < u_lu[j]))
                            q = j;
                    } while (j > 0);
                }

                /*
                 * have now found q such that d[u]!=0 and
                 * u_lu[q] is maximum
                 */
                /* store degree of new elp polynomial */
                if (l[u] > l[q] + u - q)
                    l[u + 1] = l[u];
                else
                    l[u + 1] = l[q] + u - q;

                /* form new elp(x) */
                for (i = 0; i < t2; i++)
                    elp[u + 1][i] = 0;
                for (i = 0; i <= l[q]; i++)
                    if (elp[q][i] != -1)
                        elp[u + 1][i + u - q] =
                                alpha_to[(d[u] + n - d[q] + elp[q][i]) % n];
                for (i = 0; i <= l[u]; i++) {
                    elp[u + 1][i] ^= elp[u][i];
                    elp[u][i] = index_of[elp[u][i]];
                }
            }
            u_lu[u + 1] = u - l[u + 1];

            /* form (u+1)th discrepancy */
            if (u < t2) {
                /* no discrepancy computed on last iteration */
                if (s[u + 1] != -1)
                    d[u + 1] = alpha_to[s[u + 1]];
                else
                    d[u + 1] = 0;
                for (i = 1; i <= l[u + 1]; i++)
                    if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0))
                        d[u + 1] ^= alpha_to[(s[u + 1 - i]
                                              + index_of[elp[u + 1][i]]) % n];
                /* put d[u+1] into index form */
                d[u + 1] = index_of[d[u + 1]];
            }
        } while ((u < t2) && (l[u + 1] <= t));

        u++;
        if (l[u] <= t) {/* Can correct errors */
            /* put elp into index form */
            for (i = 0; i <= l[u]; i++)
                elp[u][i] = index_of[elp[u][i]];

            /* Chien search: find roots of the error location polynomial */
            for (i = 1; i <= l[u]; i++)
                reg[i] = elp[u][i];
            count = 0;
            for (i = 1; i <= n; i++) {
                q = 1;
                for (j = 1; j <= l[u]; j++)
                    if (reg[j] != -1) {
                        reg[j] = (reg[j] + j) % n;
                        q ^= alpha_to[reg[j]];
                    }
                if (!q) {	/* store root and error
						 * location number indices */
                    root[count] = i;
                    loc[count] = n - i;
                    count++;
                }
            }
            if (count == l[u])
                /* no. roots = degree of elp hence <= t errors */
                for (i = 0; i < l[u]; i++)
                    input[loc[i]] ^= 1;
            else	/* elp has degree >t hence cannot solve */
                Serial.print("Incomplete decoding: errors detected\n");
        }
    }

    tools.convert_bin_array_to_uint8_array(&input[n - k], result, k);
    // memcpy(result, &input[n - k], sizeof(int8_t) * k);
}



int BCH::get_row() {
    return row;
}

int BCH::get_key_length() {
    return k;
}

int BCH::get_n() {
    return n;
}

void BCH::clean() {
    free(p);
    free(alpha_to);
    free(index_of);
    free(g);
}

int BCH::get_t() {
    return t;
}

int BCH::get_m() {
    return m;
}
