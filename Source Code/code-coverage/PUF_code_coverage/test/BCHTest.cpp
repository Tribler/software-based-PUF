//
// Created by Ade Setyawan on 12/12/17.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/bch/BCH.h"

using testing::Eq;
using testing::ElementsAre;
using testing::ElementsAreArray;

namespace {
    class BCHTest : public testing::Test {
    public:
        BCH bch;

        BCHTest() {
            bch = BCH();
            bch.initialize();
        }
    };

    class BarTest : public BCHTest,
                    public ::testing::WithParamInterface<const char *> {

    };
}

TEST_F(BCHTest, initialize_p) {
    ASSERT_EQ(bch.p[0], 1);
    ASSERT_EQ(bch.p[1], 1);
    ASSERT_EQ(bch.p[2], 0);
    ASSERT_EQ(bch.p[3], 0);
    ASSERT_EQ(bch.p[4], 0);
    ASSERT_EQ(bch.p[5], 0);
}

TEST_F(BCHTest, generate_gf) {
    int8_t a[] = {1, 2, 4, 8, 16, 32, 3, 6, 12, 24, 48, 35, 5, 10, 20, 40, 19, 38, 15, 30, 60, 59, 53, 41, 17, 34, 7,
                  14, 28, 56, 51, 37, 9, 18, 36, 11, 22, 44, 27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 13, 26, 52, 43, 21,
                  42, 23, 46, 31, 62, 63, 61, 57, 49, 33, 0};
    int8_t b[] = {-1, 0, 1, 6, 2, 12, 7, 26, 3, 32, 13, 35, 8, 48, 27, 18, 4, 24, 33, 16, 14, 52, 36, 54, 9, 45, 49, 38,
                  28, 41, 19, 56, 5, 62, 25, 11, 34, 31, 17, 47, 15, 23, 53, 51, 37, 44, 55, 40, 10, 61, 46, 30, 50, 22,
                  39, 43, 29, 60, 42, 21, 20, 59, 57, 58};
    for (int i = 0; i < pow(2, 6); i++) {
        ASSERT_EQ(bch.alpha_to[i], a[i]);
    }
    for (int i = 0; i < pow(2, 6); i++) {
        ASSERT_EQ(bch.index_of[i], b[i]);
    }
}

TEST_F(BCHTest, gen_poly) {
    int8_t a[] = {1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1,
                  0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < pow(2, 6); i++) {
        ASSERT_EQ(bch.g[i], a[i]);
    }
}

TEST_F(BCHTest, encode) {
    uint8_t in[] = {0xd2};
    uint8_t result[8];
    memset(result, 0, sizeof(result));
    bch.encode_bch(in, result);
    ASSERT_THAT(result, ElementsAre(0x13, 0x2a, 0x7, 0xde, 0x75, 0x85, 0xc6, 0xd2));
}

TEST_F(BCHTest, decode) {
    uint8_t in[] = {0x13, 0x2a, 0x7, 0xde, 0x75, 0x85, 0xc6, 0xd2};
    uint8_t result[1];
    memset(result, 0, sizeof(result));
    bch.decode_bch(in, result);
    ASSERT_THAT(result, ElementsAre(0xd2));
}