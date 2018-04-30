// ------------------------------------------------------------------------
//
//        File: BCH.h
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

#ifndef BCH_BCH_H
#define BCH_BCH_H

#include "Tools.h"

class BCH {
private:
    int m = 6;
    uint16_t n = 63;
    int k = 7;
    int t = 15;
    int d = 31;

    int8_t p[6];
    int8_t alpha_to[64];
    int8_t index_of[64];
    int8_t g[56];

    int8_t row = 37;

    Tools tools;

public:
    BCH();

    void
    initialize();

    void
    initialize_p();

    void
    generate_gf();

    void
    gen_poly();

    void
    encode_bch(uint8_t *input, uint8_t *result);

    void
    decode_bch(uint8_t *input, uint8_t *result);

    void decode_bch_old(int8_t *input, uint8_t *result);

    int get_row();

    int
    get_key_length();

    int
    get_n();

    int
    get_t();

    int
    get_m();

    void
    clean();
};

#endif //BCH_BCH_H
