//
// Created by Ade Setyawan on 11/12/17.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/tools/Tools.h"

using testing::Eq;
using testing::ElementsAre;

namespace {
    class ToolsTest : public testing::Test {
    public:
        Tools tools;

        ToolsTest() {
            tools = Tools();
        }
    };
}

TEST_F(ToolsTest, xor_int) {
    ASSERT_EQ(tools.xor_int(1, 1), 0);
    ASSERT_EQ(tools.xor_int(1, 0), 1);
    ASSERT_EQ(tools.xor_int(0, 1), 1);
    ASSERT_EQ(tools.xor_int(0, 0), 0);
}

TEST_F(ToolsTest, convert_uint8_to_bin_array) {
    uint8_t a = 0xFF;
    int8_t b[8];
    tools.convert_uint8_to_bin_array(a, b);
    ASSERT_THAT(b, ElementsAre(1, 1, 1, 1, 1, 1, 1, 1));
    a = 0xBC;
    memset(b, 0, sizeof(b));
    tools.convert_uint8_to_bin_array(a, b);
    ASSERT_THAT(b, ElementsAre(1, 0, 1, 1, 1, 1, 0, 0));
}

TEST_F(ToolsTest, convert_bin_array_to_uint8_array) {
    int8_t a[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    uint8_t b[2];
    memset(b, 0, sizeof(b));
    tools.convert_bin_array_to_uint8_array(a, b, 11);
    ASSERT_THAT(b, ElementsAre(0xFF, 0xe0));
}

TEST_F(ToolsTest, convert_uint8_array_to_bin_array) {
    uint8_t a[] = {0xff, 0xe0};
    int8_t b[10];
    memset(b, 0, sizeof(b));
    tools.convert_uint8_array_to_bin_array(a, b, 2);
    ASSERT_THAT(b, ElementsAre(1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    uint8_t a1[] = {0xe0, 0xff};
    memset(b, 0, sizeof(b));
    tools.convert_uint8_array_to_bin_array(a1, b, 2);
    ASSERT_THAT(b, ElementsAre(1, 1, 1, 0, 0,0,0,0, 1, 1));
    memset(b, 0, sizeof(b));
    tools.convert_uint8_array_to_bin_array(a1, b, 2);
    ASSERT_THAT(b, ElementsAre(1, 1, 1, 0, 0,0,0,0, 1, 1));
}

TEST_F(ToolsTest, combine_two_bin_array_to_uint8_array) {
    int8_t a[] = {1, 1, 1, 1, 1, 1, 1, 1};
    int8_t b[] = {1, 1, 1, 1, 1, 1, 1, 1};
    uint8_t c[2];
    tools.combine_two_bin_array_to_uint8_array(a, 8, b, 8, c);
    ASSERT_THAT(c, ElementsAre(0xff, 0xff));
    memset(c, 0, sizeof(c));
    tools.combine_two_bin_array_to_uint8_array(a, 8, b, 7, c);
    ASSERT_THAT(c, ElementsAre(0xff, 0xfe));
    memset(c, 0, sizeof(c));
    tools.combine_two_bin_array_to_uint8_array(a, 8, b, 6, c);
    ASSERT_THAT(c, ElementsAre(0xff, 0xfc));
    memset(c, 0, sizeof(c));
    tools.combine_two_bin_array_to_uint8_array(a, 6, b, 7, c);
    ASSERT_THAT(c, ElementsAre(0xff, 0xf8));
}

TEST_F(ToolsTest, ceil) {
    ASSERT_EQ(tools.ceil(4, 2), 2);
    ASSERT_EQ(tools.ceil(5, 2), 3);
    ASSERT_EQ(tools.ceil(40, 8), 5);
    ASSERT_EQ(tools.ceil(63, 8), 8);
    ASSERT_EQ(tools.ceil(64, 8), 8);
    ASSERT_EQ(tools.ceil(65, 8), 9);
}

TEST_F(ToolsTest, xor_binary_array) {
    uint8_t a[] = {0x2a};
    uint8_t b[] = {0x1c};
    uint8_t c[1];
    tools.xor_binary_array(a, b, c, 8);
    ASSERT_THAT(c, ElementsAre(0x36));
    uint8_t a1[] = {0x13, 0x2a, 0x7, 0xde, 0x75, 0x85, 0xc6, 0xd2,};
    uint8_t b1[] = {0xaa, 0x1c, 0x30, 0xd1, 0x14, 0x86, 0xbc, 0x74,};
    uint8_t c1[8];
    memset(c1, 0, sizeof(c1));
    tools.xor_binary_array(a1, b1, c1, 64);
    ASSERT_THAT(c1, ElementsAre(0xb9, 0x36, 0x37, 0xf, 0x61, 0x3, 0x7a, 0xa6));
}

TEST_F(ToolsTest, gen_key_per_row){
    uint8_t a[32];
    for(int i=0;i<32;i++){
        a[i] = 0xff;
    }
    uint8_t *b = (uint8_t *) calloc(37, sizeof(uint8_t));
    tools.gen_key_per_row(a, b);
    for(int i=0;i<32;i++){
        ASSERT_EQ(b[i], 0xfe);
    }

    uint8_t a1[] = {0xd2, 0xf, 0x56, 0x56, 0xbf, 0x43, 0x65, 0x16, 0xcd, 0xf, 0x3d, 0x2e, 0x73, 0x48, 0x51, 0xdc, 0x53,
                    0x7d, 0xf5, 0x18, 0x97, 0x48, 0x41, 0x28, 0xcc, 0xae, 0x67, 0xee, 0x13, 0x10, 0xf6, 0x9b,};
    uint8_t *b1 = (uint8_t *) calloc(37, sizeof(uint8_t));
    uint c[] = {0xd2, 0x6, 0xd4, 0xca, 0x6a, 0xfa, 0xc, 0xca, 0x16, 0x66, 0x42, 0xe6, 0xd2, 0x72, 0xcc, 0x90, 0x50, 0xee, 0x14, 0x6e, 0xde, 0xa8, 0x62, 0x2e, 0x48, 0x20, 0x4a, 0x18, 0xca, 0x72, 0x9e, 0xdc, 0x12, 0x88, 0x3c, 0xd2, 0xb0,};
    tools.gen_key_per_row(a1, b1);
    for(int i=0;i<32;i++){
        ASSERT_EQ(b1[i], c[i]);
    }
}