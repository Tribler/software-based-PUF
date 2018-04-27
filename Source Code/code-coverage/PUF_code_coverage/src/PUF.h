#include <iostream>
#include "../src/bch/BCH.h"
#include "../src/Crypto/AES.h"
#include "../src/Crypto/SHA3.h"

class PUF {
public:
    uint8_t helper_data_new[7 * 37];
    uint8_t puf_binary_new[8 * 37];

    void decode(uint8_t *key_32) {
        BCH bch;
        Tools tools;
        bch.initialize();

        uint8_t helper_data_padded_new[8 * 37];
        uint8_t xor_reproduction_new[8 * 37];
        uint8_t reconstructed_key_new[37];

        memset(helper_data_padded_new, 0, sizeof(helper_data_padded_new));
        memset(xor_reproduction_new, 0, sizeof(xor_reproduction_new));
        memset(reconstructed_key_new, 0, sizeof(reconstructed_key_new));

        int row = 37;

        /************************************
         ****** REPRODUCTION PROCEDURE ******
         ************************************/
        /******************** ASSERT HELPER DATA PADDED**********************/
        for (int i = 0; i < row; i++) {
            memcpy(&helper_data_padded_new[i * 8], &helper_data_new[i * 7], 7 * sizeof(uint8_t));
        }

        /******************** ASSERT XOR REPRODUCTION **********************/
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < 8; j++) {
                xor_reproduction_new[i * 8 + j] = puf_binary_new[i * 8 + j] ^ helper_data_padded_new[i * 8 + j];
            }
        }

        /******************** ASSERT RECONSTRUCTED KEY **********************/
        for (int i = 0; i < row; i++) {
            bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
        }

        tools.convert_key_back(reconstructed_key_new, key_32);
    }

    void derive_new_key(string user_password, uint8_t* final_key, uint8_t* key_32) {
        SHA3_256 sha3_256;

        sha3_256.resetHMAC(key_32, 32);
        sha3_256.update(reinterpret_cast<const uint8_t*>(&user_password[0]), user_password.length());
        sha3_256.finalizeHMAC(key_32, 32, final_key, 32);
    }

    void encrypt_test(uint8_t* final_key, uint8_t *plain, uint8_t *result) {
        AES256 aes256;
        aes256.setKey(final_key, 32);
        aes256.encryptBlock(result, plain);
    }

    void decrypt_test(uint8_t* final_key, uint8_t *cypher, uint8_t *decrypted){
        AES256 aes256;
        aes256.setKey(final_key, 32);
        aes256.decryptBlock(decrypted, cypher);
    }
};

