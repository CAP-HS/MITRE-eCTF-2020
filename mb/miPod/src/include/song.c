#include <stdio.h>
/*#include "aes.h"
#include "ecdh.h"
#include "utils.h"
#include "constantsA.h"*/
#include "song.h"
//#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>


int encrypt_song (uint8_t* key, uint8_t* raw_song, uint8_t* enc_song)
{
    //
    // raw song and enc song is of size 16 bytes while
    // key is of size 16-bytes
    //struct tc_aes_key_sched_struct s;
    int result = TC_PASS;

    struct tc_aes_key_sched_struct s;
    //testing to make sure key schedule was sucessful    
    //TC_PRINT("AES128 %s (key schedule):\n", __func__);
	// if (tc_aes128_set_encrypt_key(&s, key) == 0) 
    // {
	// 	TC_ERROR("AES128 ENC %s key schedule failed.\n", __func__);
	// 	result = TC_FAIL;
	// 	//goto exitTest1;
	// }
    // result = check_result(1, expected.words, sizeof(expected.words), s.words,
    //             sizeof(s.words));

    tc_aes128_set_encrypt_key(&s, key);
    //TC_PRINT("pass key schedule");

    // exitTest1:
    // 	TC_END_RESULT(result);
    // 	return result;

    // TC_PRINT("AES128 %s (NIST encryption test):\n", __func__);

	// (void)tc_aes128_set_encrypt_key(&s, key);
	// if (tc_aes_encrypt(enc_song, raw_song, &s) == 0) 
    // {
	// 	TC_ERROR("AES128 %s (encryption) failed.\n",
	// 		 __func__);
	// 	result = TC_FAIL;
	// 	goto exitTest2;
	// }

	//result = check_result(2, expected, sizeof(expected), enc_song, sizeof(enc_song));

    // exitTest2:
    //     TC_END_RESULT(result);

    //     return result;

        tc_aes_encrypt(enc_song, raw_song, &s);

};


int decrypt_song(uint8_t* key, uint8_t* raw_song, uint8_t* enc_song)
{
    int result = TC_PASS;
     struct tc_aes_key_sched_struct s;
     tc_aes128_set_decrypt_key(&s, key);
     tc_aes_decrypt(raw_song,enc_song,&s);
     		if (tc_aes_decrypt(raw_song, enc_song, &s) == 0) {
			TC_ERROR("aes_decrypt failed\n");
			result = TC_FAIL;
		}
};

// int key_gen(uint8_t * pub_key, uint8_t * priv_key)
// {
//     // this program generates keys
//     int status;
//     status = ecdh_generate_keys(pub_key, priv_key);
//     return status;
// };

// int keyshared(const uint8_t* private_key, const uint8_t* others_pub, uint8_t* sharedkey)
// {
//     int status;
//     status = ecdh_shared_secret(private_key,others_pub,sharedkey);
//     return status;
// };
