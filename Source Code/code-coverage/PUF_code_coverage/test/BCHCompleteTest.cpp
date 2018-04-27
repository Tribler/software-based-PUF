//
// Created by Ade Setyawan on 13/12/17.
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../src/bch/BCH.h"

using testing::Eq;
using testing::ElementsAre;
using testing::ElementsAreArray;

namespace {
    class BCHCompleteTest : public testing::Test {
    public:
        BCH bch;
        Tools tools;

        BCHCompleteTest() {
            bch = BCH();
            bch.initialize();
            tools = Tools();
        }
    };

    class BarTest : public BCHCompleteTest,
                    public ::testing::WithParamInterface<const char *> {

    };
}

uint8_t puf_binary[] = {0x4C, 0x97, 0xB2, 0xC6, 0xC1, 0x2E, 0x9A, 0xF6, 0x57, 0x2D, 0x9B, 0xCA, 0xF3, 0x3E, 0xA6, 0x5B,
                        0x63, 0xE5, 0x29, 0xF9, 0xBC, 0x60, 0x1A, 0xFF, 0xAD, 0x69, 0x3F, 0x10, 0x48, 0xED, 0x7C, 0xEB,
                        0xD5, 0xF, 0xFF, 0xAE, 0xF2, 0xEA, 0xFB, 0xAE, 0x1D, 0xCE, 0x9C, 0xA9, 0xDA, 0x91, 0xD6, 0xD8,
                        0x89, 0x96, 0xF7, 0xD6, 0xC8, 0xDF, 0xA, 0xE3, 0x7B, 0xDF, 0xD6, 0xDF, 0xCE, 0xE5, 0x15, 0x50,
                        0xB1, 0x7E, 0x4F, 0x43, 0x3A, 0xE3, 0xD2, 0x6C, 0x5F, 0x6E, 0xC3, 0x40, 0xBA, 0x2C, 0x43, 0xD0,
                        0xAB, 0xA3, 0xCF, 0x2, 0x1E, 0x40, 0xFE, 0xBB, 0xDE, 0xEC, 0x31, 0xDF, 0x6B, 0x4F, 0xB8, 0x43,
                        0xF9, 0x36, 0x8C, 0x65, 0x34, 0xC7, 0x58, 0x0, 0xC6, 0x27, 0x2B, 0x70, 0x94, 0x30, 0x4C, 0xB6,
                        0x27, 0x75, 0x1F, 0xE7, 0xA7, 0x67, 0xB7, 0xA6, 0x25, 0xC3, 0xF2, 0xF7, 0xA3, 0x37, 0x54, 0xFE,
                        0x3A, 0x94, 0x14, 0x56, 0xEC, 0xF2, 0xE3, 0xA7, 0xE9, 0xBE, 0x9E, 0x3B, 0x7E, 0x91, 0x7A, 0xB6,
                        0x8F, 0xCC, 0xFD, 0x5E, 0x2F, 0xF0, 0x69, 0x5C, 0xCA, 0xEC, 0x85, 0x62, 0x6D, 0x89, 0x4E, 0x3C,
                        0x3C, 0x65, 0xFF, 0xFB, 0x65, 0xB7, 0x6A, 0x77, 0xE9, 0x4F, 0xDF, 0x46, 0xD2, 0x36, 0xB0, 0x46,
                        0xCD, 0x7D, 0xE3, 0x54, 0x57, 0x2F, 0xA6, 0xD0, 0x4F, 0xDB, 0x13, 0x8F, 0x24, 0x22, 0x4A, 0x56,
                        0xA7, 0xC3, 0x3E, 0x85, 0xCB, 0x9F, 0xC8, 0x29, 0x46, 0x1C, 0x1E, 0x85, 0xE5, 0xCC, 0xB6, 0x80,
                        0xAF, 0xF7, 0x86, 0xFF, 0x76, 0x3E, 0x3C, 0xA7, 0x3E, 0x28, 0x4, 0x97, 0x6F, 0x6F, 0x38, 0xEE,
                        0xEA, 0x5, 0x71, 0x8B, 0xA4, 0xAA, 0x76, 0xAC, 0xE9, 0xE5, 0xE5, 0xAD, 0x99, 0xA1, 0xF9, 0xC9,
                        0xF, 0xFB, 0x87, 0xF1, 0x54, 0x30, 0x99, 0x76, 0xF8, 0xB0, 0x9E, 0xCC, 0x5F, 0xF6, 0x73, 0x91,
                        0xA6, 0x22, 0x7, 0x62, 0x2E, 0xF4, 0x15, 0x9C, 0xA6, 0x73, 0x43, 0xEF, 0x6F, 0xB1, 0x49, 0x53,
                        0xA5, 0xC1, 0xE3, 0xFE, 0x5E, 0xC8, 0x38, 0x2F, 0x2, 0x99, 0xC4, 0xAD, 0xE0, 0x1F, 0x81, 0xF5,
                        0x6A, 0xAC, 0xE, 0xE, 0x0, 0x0, 0x0, 0x0,};

//uint8_t key_256[] = {0xd2, 0xf, 0x56, 0x56, 0xbf, 0x43, 0x65, 0x16, 0xcd, 0xf, 0x3d, 0x2e, 0x73, 0x48, 0x51, 0xdc,
//                     0x53, 0x7d, 0xf5, 0x18, 0x97, 0x48, 0x41, 0x28, 0xcc, 0xae, 0x67, 0xee, 0x13, 0x10, 0xf6,
//                     0x9b,};

uint8_t key_256[] = {0x4C, 0x97, 0xB2, 0xC6, 0xC1, 0x2E, 0x9A, 0xF6, 0x57, 0x2D, 0x9B, 0xCA, 0xF3, 0x3E, 0xA6, 0x5B,
                     0x63, 0xE5, 0x29, 0xF9, 0xBC, 0x60, 0x1A, 0xFF, 0xAD, 0x69, 0x3F, 0x10, 0x48, 0xED, 0x7C, 0xEB,};

TEST_F(BCHCompleteTest, encode_decode_single_key) {
    uint8_t in[1];
    uint8_t out[1];
    uint8_t result[8];
    memset(in, 0, sizeof(in));
    memset(out, 0, sizeof(out));
    memset(result, 0, sizeof(result));
    for (int i = 0; i < 128; i++) {
        in[0] = i << 1;
        bch.encode_bch(in, result);
        bch.decode_bch(result, out);
        ASSERT_EQ(in[0], out[0]);
        memset(in, 0, sizeof(in));
        memset(out, 0, sizeof(out));
        memset(result, 0, sizeof(result));
    }
}

TEST_F(BCHCompleteTest, encode_decode_multi_key_no_error) {
    int length = 1;
    uint8_t *in = (uint8_t *) calloc(length, sizeof(uint8_t));
    uint8_t *out = (uint8_t *) calloc(length, sizeof(uint8_t));
    uint8_t *result = (uint8_t *) calloc(length * 8, sizeof(uint8_t));;
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < length; j++) {
            in[j] = (i + j) << 1;
            bch.encode_bch(&in[j], &result[j]);
            bch.decode_bch(&result[j], &out[j]);
            ASSERT_EQ(in[j], out[j]);
        }
        memset(in, 0, sizeof(in));
        memset(out, 0, sizeof(out));
        memset(result, 0, sizeof(result));
    }
}

TEST_F(BCHCompleteTest, encode_decode_multi_key_with_error) {
    int length = 3;
    uint8_t *in = (uint8_t *) calloc(length, sizeof(uint8_t));
    uint8_t *out = (uint8_t *) calloc(length, sizeof(uint8_t));
    uint8_t *result = (uint8_t *) calloc(length * 8, sizeof(uint8_t));;
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < length; j++) {
            in[j] = (i + j) << 1;
            bch.encode_bch(&in[j], &result[j]);
            bch.decode_bch(&result[j], &out[j]);
            ASSERT_EQ(in[j], out[j]);
        }
        memset(in, 0, sizeof(in));
        memset(out, 0, sizeof(out));
        memset(result, 0, sizeof(result));
    }
}

//TEST_F(BCHCompleteTest, complete) {
//    int row = bch.get_row();
//    int n = bch.get_n();
//    int k = bch.get_key_length();
//    int index, shift, ii;
//
//    int8_t puf_binary_default[] = {
//            1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
//            0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1,
//            0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1,
//            1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
//            1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
//            0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1,
//            0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0,
//            0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0,
//            0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
//            1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1,
//            1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0,
//            1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1,
//            1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
//            0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
//            0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0,
//            1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
//            0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,
//            0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0,
//            0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,
//            0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//            0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
//            1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1,
//            1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0,
//            0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1,
//            0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0,
//            0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
//            0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
//            1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0,
//            0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0,
//            1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
//            1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1,
//            1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0,
//            1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,
//            1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1,
//            0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,
//            0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1,
//            1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0,
//            1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//            0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0,
//            1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
//            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1,
//            1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
//            1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0,
//            0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1,
//            0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1,
//            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0,
//            1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0,
//            1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0,
//            1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
//            0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0,
//            1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0,
//            1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1,
//            1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0,
//            0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1,
//            1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1,
//            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
//            1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1,
//            0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0,
//            1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0,
//            1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0,
//            0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0,
//            0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1,
//            1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1,
//            1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1,
//            0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0,
//            1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
//            1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0,
//            1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0,
//            0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1,
//            1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1,
//            0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0,
//            1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1,
//            1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1,
//            1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
//    };
//
//    int8_t key[] = {   1, 1, 0, 1, 0, 0, 1,
//                       0, 0, 0, 0, 0, 1, 1,
//                       1, 1, 0, 1, 0, 1, 0,
//                       1, 1, 0, 0, 1, 0, 1,
//                       0, 1, 1, 0, 1, 0, 1,
//                       1, 1, 1, 1, 1, 0, 1,
//                       0, 0, 0, 0, 1, 1, 0,
//                       1, 1, 0, 0, 1, 0, 1,
//                       0, 0, 0, 1, 0, 1, 1,
//                       0, 1, 1, 0, 0, 1, 1,
//                       0, 1, 0, 0, 0, 0, 1,
//                       1, 1, 1, 0, 0, 1, 1,
//                       1, 1, 0, 1, 0, 0, 1,
//                       0, 1, 1, 1, 0, 0, 1,
//                       1, 1, 0, 0, 1, 1, 0,
//                       1, 0, 0, 1, 0, 0, 0,
//                       0, 1, 0, 1, 0, 0, 0,
//                       1, 1, 1, 0, 1, 1, 1,
//                       0, 0, 0, 1, 0, 1, 0,
//                       0, 1, 1, 0, 1, 1, 1,
//                       1, 1, 0, 1, 1, 1, 1,
//                       1, 0, 1, 0, 1, 0, 0,
//                       0, 1, 1, 0, 0, 0, 1,
//                       0, 0, 1, 0, 1, 1, 1,
//                       0, 1, 0, 0, 1, 0, 0,
//                       0, 0, 1, 0, 0, 0, 0,
//                       0, 1, 0, 0, 1, 0, 1,
//                       0, 0, 0, 1, 1, 0, 0,
//                       1, 1, 0, 0, 1, 0, 1,
//                       0, 1, 1, 1, 0, 0, 1,
//                       1, 0, 0, 1, 1, 1, 1,
//                       1, 1, 0, 1, 1, 1, 0,
//                       0, 0, 0, 1, 0, 0, 1,
//                       1, 0, 0, 0, 1, 0, 0,
//                       0, 0, 1, 1, 1, 1, 0,
//                       1, 1, 0, 1, 0, 0, 1,
//                       1, 0, 1, 1, 0, 0, 0};
//
//    uint8_t *key_per_row = (uint8_t *) calloc(row, sizeof(uint8_t));
//    tools.gen_key_per_row(key_256, key_per_row);
//
//    int k_length_bit = row;//tools.ceil(k*row,8);
//    int n_length_bit = row * 8;//tools.ceil(n*row,8);
//
//    /******************** ASSERT KEY **********************/
//    index = 0;
//    shift = 0;
//    for (int i = 0; i < 256; i++) {
//        shift = 7 - (i % 7);
//        ASSERT_EQ(key[i], key_per_row[index] >> shift & 0x1);
//        if ((i + 1) % 7 == 0) {
//            index++;
//        }
//    }
//
//    /******************** ASSERT DEFAULT BINARY **********************/
//    uint8_t *puf_binary_new = (uint8_t *) calloc(row * 8, sizeof(uint8_t));
//    tools.convert_bin_array_to_uint8_array(puf_binary_default, puf_binary_new, row * 64);
//    index = 0;
//    shift = 0;
//    for (int i = 0; i < row * 64; i++) {
//        shift = 7 - (i % 8);
//        ASSERT_EQ(puf_binary_default[i], puf_binary_new[index] >> shift & 0x1);
//        if ((i + 1) % 8 == 0)
//            index++;
//    }
//
//    /************************************
//     ****** GENERATION PROCEDURE ******
//     ************************************/
//    bch.initialize();               /* Read m */
//
//    /******************** ASSERT ENCODED **********************/
//    int8_t *encoded = (int8_t *) calloc(n * row, sizeof(int8_t));
//    uint8_t *encoded_new = (uint8_t *) malloc(8 * row * sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        bch.encode_bch_old(&key[i * k], &encoded[i * n]);           /* encode data */
//        bch.encode_bch(&key_per_row[i], &encoded_new[i * 8]);           /* encode data */
//    }
//    index = 0;
//    shift = 0;
//    ii = 0;
//    for (int i = 0; i < row * 63; i++) {
//        shift = 7 - (ii % 8);
//        ASSERT_EQ(encoded[i], encoded_new[index] >> shift & 0x1);
//        ii++;
//        if (ii % 8 == 0)
//            index++;
//        if (ii % 63 == 0) {
//            index++;
//            ii = 0;
//        }
//    }
//
//    /******************** ASSERT XOR ENROLL **********************/
//    int8_t *xor_enroll = (int8_t *) calloc(n * row, sizeof(int8_t));
//    uint8_t *xor_enroll_new = (uint8_t *) calloc(8 * row, sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        tools.xor_binary_array(&encoded_new[i * 8], &puf_binary_new[i * 8], &xor_enroll_new[i * 8], 63);
//        memcpy(&xor_enroll[i * 63], tools.xor_array(&encoded[i * 63], &puf_binary_default[i * 64], n),
//               63 * sizeof(int8_t));
//    }
//    index = 0;
//    shift = 0;
//    ii = 0;
//    for (int i = 0; i < row * 63; i++) {
//        shift = 7 - (ii % 8);
//        ASSERT_EQ(xor_enroll[i], xor_enroll_new[index] >> shift & 0x1);
//        ii++;
//        if (ii % 8 == 0)
//            index++;
//        if (ii % 63 == 0) {
//            index++;
//            ii = 0;
//        }
//    }
//
//    /******************** ASSERT HELPER DATA **********************/
//    int8_t *helper_data = (int8_t *) calloc((n - k) * row, sizeof(int8_t));
//    uint8_t *helper_data_new = (uint8_t *) calloc(row * 8, sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        memcpy(&helper_data[i * (n - k)], &xor_enroll[i * n], (n - k) * sizeof(int8_t));
//        memcpy(&helper_data_new[i * 7], &xor_enroll_new[i * 8], 7 * sizeof(uint8_t));
//    }
//    index = 0;
//    shift = 0;
//    ii = 0;
//    for (int i = 0; i < row * 56; i++) {
//        shift = 7 - (ii % 8);
//        ASSERT_EQ(helper_data[i], helper_data_new[index] >> shift & 0x1);
//        ii++;
//        if (ii % 8 == 0)
//            index++;
//    }
//
//    /************************************
//     ****** REPRODUCTION PROCEDURE ******
//     ************************************/
//    /******************** ASSERT HELPER DATA PADDED**********************/
//    int8_t *helper_data_padded = (int8_t *) calloc(n * row, sizeof(int8_t));
//    uint8_t *helper_data_padded_new = (uint8_t *) calloc(row * 8, sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        memcpy(&helper_data_padded[i * n], &helper_data[i * (n - k)], (n - k) * sizeof(int8_t));
//        memcpy(&helper_data_padded_new[i * 8], &helper_data_new[i * 7],
//               7 * sizeof(uint8_t));
//    }
//    index = 0;
//    shift = 0;
//    ii = 0;
//    for (int i = 0; i < row * 63; i++) {
//        shift = 7 - (ii % 8);
//        ASSERT_EQ(helper_data_padded[i], helper_data_padded_new[index] >> shift & 0x1);
//        ii++;
//        if (ii % 8 == 0)
//            index++;
//        if (ii % 63 == 0) {
//            index++;
//            ii = 0;
//        }
//    }
//
//    /******************** ASSERT XOR REPRODUCTION **********************/
//    int8_t *xor_reproduction = (int8_t *) calloc(n * row, sizeof(int8_t));
//    uint8_t *xor_reproduction_new = (uint8_t *) calloc(n_length_bit, sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        memcpy(&xor_reproduction[i * 63], tools.xor_array(&helper_data_padded[i * 63], &puf_binary_default[i * 64], n),
//               63 * sizeof(int8_t));
//        tools.xor_binary_array(&puf_binary_new[i * 8], &helper_data_padded_new[i * 8], &xor_reproduction_new[i * 8],
//                               64);
//    }
////    tools.xor_binary_array(puf_binary_new, helper_data_padded_new, xor_reproduction_new, 64*row);
//    index = 0;
//    shift = 0;
//    ii = 0;
//    for (int i = 0; i < row * 63; i++) {
//        shift = 7 - (ii % 8);
////        printf("%d%d ", xor_reproduction[i], xor_reproduction_new[index] >> shift & 0x1);
//        ASSERT_EQ(xor_reproduction[i], xor_reproduction_new[index] >> shift & 0x1);
//        ii++;
//        if (ii % 8 == 0)
//            index++;
//        if (ii % 63 == 0) {
//            index++;
//            ii = 0;
//        }
//    }
//
//    /******************** ASSERT RECONSTRUCTED KEY **********************/
//    int8_t *reconstructed_key = (int8_t *) malloc(sizeof(int8_t) * k * row);
//    uint8_t *reconstructed_key_new = (uint8_t *) calloc(k_length_bit, sizeof(uint8_t));
//    for (int i = 0; i < row; i++) {
//        bch.decode_bch_old(&xor_reproduction[i * n], &reconstructed_key[i * k]);
//        bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
//    }
//    index = 0;
//    shift = 0;
//    for (int i = 0; i < row * k; i++) {
//        shift = 7 - (i % 7);
//        ASSERT_EQ(reconstructed_key[i], reconstructed_key_new[index] >> shift & 0x1);
//        if ((i + 1) % 7 == 0)
//            index++;
//    }
//
//    /************************************
//     ********** FREE MEMORY *************
//     ************************************/
//    free(key_per_row);
//    free(puf_binary_new);
//    free(encoded);
//    free(encoded_new);
//    free(xor_enroll);
//    free(xor_enroll_new);
//    free(xor_reproduction);
//    free(xor_reproduction_new);
//    free(helper_data);
//    free(helper_data_new);
//    free(helper_data_padded);
//    free(helper_data_padded_new);
//    free(reconstructed_key);
//    free(reconstructed_key_new);
//}

TEST_F(BCHCompleteTest, encode_decode_complete) {
    int row = 37;
    int index, shift, ii;

    uint8_t *key_per_row = (uint8_t *) calloc(row, sizeof(uint8_t));
    tools.gen_key_per_row(key_256, key_per_row);
//    uint8_t *key_32 = (uint8_t *) calloc(32, sizeof(uint8_t));
//    memcpy(key_32, puf_binary, 32 * sizeof(uint8_t));
//    tools.gen_key_per_row(key_32, key_per_row);

    int k_length_bit = row;//tools.ceil(k*row,8);
    int n_length_bit = row * 8;//tools.ceil(n*row,8);

    /************************************
     ****** GENERATION PROCEDURE ******
     ************************************/
    bch.initialize();               /* Read m */

    /******************** ASSERT ENCODED **********************/
    uint8_t *encoded_new = (uint8_t *) malloc(8 * row * sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        bch.encode_bch(&key_per_row[i], &encoded_new[i * 8]);           /* encode data */
    }
//    for (int i = 0; i < row * 8; i++) {
//        printf("0x%x, ", encoded_new[i]);
//        if ((i + 1) % 8 == 0)
//            printf("\n");
//    }
//    printf("------------------------\n\n");

    /******************** ASSERT XOR ENROLL **********************/
    uint8_t *xor_enroll_new = (uint8_t *) calloc(8 * row, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        tools.xor_binary_array(&encoded_new[i * 8], &puf_binary[i * 8], &xor_enroll_new[i * 8], 63);
    }

    /******************** ASSERT HELPER DATA **********************/
    uint8_t *helper_data_new = (uint8_t *) calloc(row * 7, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        memcpy(&helper_data_new[i * 7], &xor_enroll_new[i * 8], 7 * sizeof(uint8_t));
    }
    for (int i = 0; i < row * 7; i++) {
        printf("0x%x, ", helper_data_new[i]);
        if ((i + 1) % 7 == 0)
            printf("\n");
    }

    /************************************
     ****** REPRODUCTION PROCEDURE ******
     ************************************/
    /******************** ASSERT HELPER DATA PADDED**********************/
    uint8_t *helper_data_padded_new = (uint8_t *) calloc(row * 8, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        memcpy(&helper_data_padded_new[i * 8], &helper_data_new[i * 7],
               7 * sizeof(uint8_t));
    }

    /******************** ASSERT XOR REPRODUCTION **********************/
    uint8_t *xor_reproduction_new = (uint8_t *) calloc(n_length_bit, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        tools.xor_binary_array(&puf_binary[i * 8], &helper_data_padded_new[i * 8], &xor_reproduction_new[i * 8],
                               64);
    }

    /******************** ASSERT RECONSTRUCTED KEY **********************/
    uint8_t *reconstructed_key_new = (uint8_t *) calloc(k_length_bit, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
    }
    for (int i = 0; i < row; i++) {
        ASSERT_EQ(key_per_row[i], reconstructed_key_new[i]);
    }

    /************************************
     ********** FREE MEMORY *************
     ************************************/
    free(key_per_row);
    free(encoded_new);
    free(xor_enroll_new);
    free(xor_reproduction_new);
    free(helper_data_new);
    free(helper_data_padded_new);
    free(reconstructed_key_new);
}

TEST_F(BCHCompleteTest, decode_complete) {
    int row =37;

    uint8_t helper_data_default[] = {0x18, 0x98, 0xe, 0x2d, 0xca, 0xa3, 0x3e,
                                     0xec, 0x18, 0xe5, 0xc2, 0x91, 0xa0, 0x28,
                                     0x9, 0x19, 0x39, 0x3c, 0x81, 0x7c, 0x8c,
                                     0x83, 0x5f, 0xae, 0x22, 0xe8, 0x90, 0x9b,
                                     0x44, 0x3d, 0x5f, 0xd3, 0x15, 0xb2, 0xa7,
                                     0x2c, 0x81, 0xdb, 0x8c, 0x61, 0xa4, 0xa8,
                                     0x13, 0x29, 0xf3, 0xe7, 0x87, 0x98, 0x2f,
                                     0xc4, 0xdb, 0xe7, 0x90, 0x89, 0xc0, 0xae,
                                     0x7f, 0xce, 0xf7, 0x99, 0x7e, 0x29, 0x53,
                                     0x5c, 0x81, 0xf9, 0x82, 0x59, 0x45, 0x50,
                                     0xeb, 0x58, 0x1, 0xb2, 0xa6, 0x9a, 0xba,
                                     0x75, 0x1c, 0x72, 0xcb, 0x9f, 0x3d, 0xe3,
                                     0x8c, 0xb3, 0x4a, 0xb7, 0x12, 0x93, 0x57,
                                     0x36, 0x64, 0x3f, 0x84, 0xe6, 0x6b, 0xff,
                                     0x88, 0xb4, 0x13, 0xb4, 0x76, 0xae, 0xd9,
                                     0xd6, 0x6f, 0xdc, 0xc1, 0x32, 0x5, 0xf4,
                                     0xeb, 0x5d, 0x7a, 0x9b, 0xb3, 0x70, 0xfb,
                                     0x5, 0x6b, 0x66, 0x1a, 0xf4, 0xeb, 0x43,
                                     0x5a, 0x34, 0xdc, 0xd4, 0x55, 0xc9, 0x44,
                                     0x39, 0x40, 0xab, 0x54, 0xfc, 0xbb, 0xee,
                                     0xd4, 0x81, 0x48, 0x9d, 0xca, 0x76, 0x66,
                                     0x4a, 0xdd, 0x2, 0xdc, 0x6d, 0x32, 0x81,
                                     0x61, 0x53, 0xd5, 0xc5, 0x65, 0x8f, 0xdb,
                                     0xa4, 0xd0, 0x9e, 0x2b, 0x68, 0x8a, 0x55,
                                     0x1f, 0x19, 0x7a, 0x4f, 0x4a, 0x68, 0x55,
                                     0x6f, 0xf4, 0xfa, 0x32, 0x83, 0x63, 0x77,
                                     0x2d, 0xef, 0x21, 0x5c, 0xe4, 0xe3, 0xa6,
                                     0x7, 0x5, 0xdd, 0x3c, 0x9f, 0x2c, 0x2c,
                                     0xe1, 0x88, 0xd5, 0xc7, 0xc, 0xb5, 0xf,
                                     0xa5, 0x4d, 0xfa, 0xd4, 0x4f, 0xb6, 0xe2,
                                     0x7, 0x99, 0x19, 0x7f, 0x1f, 0x46, 0xf3,
                                     0x3d, 0x8d, 0x82, 0x5a, 0xb3, 0x23, 0x8b,
                                     0xea, 0x8a, 0x18, 0x1b, 0xf8, 0xe3, 0xe,
                                     0x93, 0xd, 0x4b, 0x8d, 0xf1, 0x3f, 0x2,
                                     0x64, 0xcd, 0xb0, 0x2f, 0x97, 0xa6, 0xf5,
                                     0xb2, 0x21, 0x1e, 0xe9, 0x2a, 0x9e, 0x76,
                                     0x36, 0xc1, 0x2c, 0x6b, 0x40, 0xfb, 0xce,
    };

    uint8_t *key_per_row = (uint8_t *) calloc(row, sizeof(uint8_t));
    tools.gen_key_per_row(key_256, key_per_row);
//    uint8_t *key_per_row = (uint8_t *) calloc(row, sizeof(uint8_t));
//    uint8_t *key_32 = (uint8_t *) calloc(32, sizeof(uint8_t));
//    memcpy(key_32, puf_binary, 32 * sizeof(uint8_t));
//    tools.gen_key_per_row(key_32, key_per_row);

    int k_length_bit = row;//tools.ceil(k*row,8);
    int n_length_bit = row * 8;//tools.ceil(n*row,8);

    /************************************
     ****** REPRODUCTION PROCEDURE ******
     ************************************/
    /******************** ASSERT HELPER DATA PADDED**********************/
    uint8_t *helper_data_padded_new = (uint8_t *) calloc(row * 8, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        memcpy(&helper_data_padded_new[i * 8], &helper_data_default[i * 7],
               7 * sizeof(uint8_t));
//        printf("%x, %x, %x, %x, %x, %x, %x, %x, \n", helper_data_padded_new[i * 8], helper_data_padded_new[i * 8 + 1], helper_data_padded_new[i * 8 + 2], helper_data_padded_new[i * 8 + 3], helper_data_padded_new[i * 8 + 4], helper_data_padded_new[i * 8 + 5], helper_data_padded_new[i * 8 + 6], helper_data_padded_new[i * 8 + 7]);
    }

//    printf("\nJEMBUT\n");

    /******************** ASSERT XOR REPRODUCTION **********************/
    uint8_t *xor_reproduction_new = (uint8_t *) calloc(n_length_bit, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        tools.xor_binary_array(&puf_binary[i * 8], &helper_data_padded_new[i * 8], &xor_reproduction_new[i*8],
                               64);
//        printf("%x, %x, %x, %x, %x, %x, %x, %x, \n", xor_reproduction_new[i * 8], xor_reproduction_new[i * 8 + 1], xor_reproduction_new[i * 8 + 2], xor_reproduction_new[i * 8 + 3], xor_reproduction_new[i * 8 + 4], xor_reproduction_new[i * 8 + 5], xor_reproduction_new[i * 8 + 6], xor_reproduction_new[i * 8 + 7]);
//        printf("%x, %x, %x, %x, %x, %x, %x, %x, \n", helper_data_padded_new[i * 8], helper_data_padded_new[i * 8 + 1], helper_data_padded_new[i * 8 + 2], helper_data_padded_new[i * 8 + 3], helper_data_padded_new[i * 8 + 4], helper_data_padded_new[i * 8 + 5], helper_data_padded_new[i * 8 + 6], helper_data_padded_new[i * 8 + 7]);
//        printf("%x, %x, %x, %x, %x, %x, %x, %x, \n", puf_binary[i * 8], puf_binary[i * 8 + 1], puf_binary[i * 8 + 2], puf_binary[i * 8 + 3], puf_binary[i * 8 + 4], puf_binary[i * 8 + 5], puf_binary[i * 8 + 6], puf_binary[i * 8 + 7]);
//        printf("------------------------------------------------------------------------------------\n");
//        uint8_t temp[8];
//        memset(temp, 0, sizeof(temp));
//        tools.xor_binary_array(&puf_binary[i * 8], &helper_data_padded_new[i * 8], temp,
//                               64);
        /******** CREATE ERROR ********/
//        int *a = tools.generate_random(0, 63, 15);
//        uint8_t *b = (uint8_t *) calloc(8, sizeof(uint8_t));
//        tools.create_random_bit(a, 8, 63, b);
//        tools.xor_binary_array(b, temp, &xor_reproduction_new[i * 8], 64);
//        free(a);
//        free(b);
    }


    /******************** ASSERT RECONSTRUCTED KEY **********************/
    uint8_t *reconstructed_key_new = (uint8_t *) calloc(k_length_bit, sizeof(uint8_t));
    for (int i = 0; i < row; i++) {
        bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
    }
    for (int i = 0; i < row; i++) {
//        printf("%d - ", i);
//        tools.print_binary(reconstructed_key_new[i]);
//        tools.print_binary(key_per_row[i]);
//        printf(" %d", tools.difference_bin_array(&reconstructed_key_new[i], &key_per_row[i], 8));
//        printf("\n");
        ASSERT_EQ(reconstructed_key_new[i], key_per_row[i]);
    }

    /************************************
     ********** FREE MEMORY *************
     ************************************/
    free(key_per_row);
    free(xor_reproduction_new);
    free(helper_data_padded_new);
    free(reconstructed_key_new);
}