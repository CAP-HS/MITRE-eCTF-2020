#ifndef __SONG_H__
#define __SONG_H__

#include "aes.h"
/*#include "ecdh.h"
#include "utils.h"
#include "constantsA.h"*/

// start code from tinycrypt
#define PRINT_DATA(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define PRINT_LINE                                                     \
	PRINT_DATA(                                                        \
		"============================================================" \
		"=======\n")


#define FAIL "FAIL"
#define PASS "PASS"
#define FMT_ERROR "%s - %s@%d. "

/* TC_ here stands for 'Test Case' not 'TinyCrypt' */
#define TC_PASS 0
#define TC_FAIL 1

#define TC_ERROR(fmt, ...)                               \
	do {                                                 \
		PRINT_DATA(FMT_ERROR, FAIL, __func__, __LINE__); \
		PRINT_DATA(fmt, ##__VA_ARGS__);                  \
	} while (0)

#define TC_PRINT(fmt, ...) PRINT_DATA(fmt, ##__VA_ARGS__)
#define TC_START(name) PRINT_DATA("tc_start() - %s\n", name)
#define TC_END(result, fmt, ...) PRINT_DATA(fmt, ##__VA_ARGS__)

/* prints result and the function name */
#define TC_END_RESULT(result)                           \
	do {                                                \
		PRINT_LINE;                                     \
		TC_END(result, "%s - %s.\n",                    \
			result == TC_PASS ? PASS : FAIL, __func__); \
	} while (0)

#define TC_END_REPORT(result)                               \
	do {                                                    \
		PRINT_LINE;                                         \
		TC_END(result,                                      \
			"PROJECT EXECUTION %s\n",               \
			result == TC_PASS ? "SUCCESSFUL" : "FAILED");   \
	} while (0)

static inline void show_str(const char *label, const uint8_t *s, size_t len)
{
        unsigned int i;

        TC_PRINT("%s = ", label);
        for (i = 0; i < (unsigned int) len; ++i) {
                TC_PRINT("%02x", s[i]);
        }
        TC_PRINT("\n");
}

static inline void fatal(unsigned int testnum, const void *expected, size_t expectedlen,
           const void *computed, size_t computedlen)
{

        TC_ERROR("\tTest #%d Failed!\n", testnum);
        show_str("\t\tExpected", expected, expectedlen);
        show_str("\t\tComputed  ", computed, computedlen);
        TC_PRINT("\n");
}

static inline unsigned int check_result(unsigned int testnum, const void *expected, size_t expectedlen,
                   const void *computed, size_t computedlen)
{
	unsigned int result = TC_PASS;

        if (expectedlen != computedlen) {
                TC_ERROR("The length of the computed buffer (%zu)", computedlen);
                TC_ERROR("does not match the expected length (%zu).", expectedlen);
                result = TC_FAIL;
        } else if (memcmp(computed, expected, computedlen) != 0) {
                fatal(testnum, expected, expectedlen, computed, computedlen);
                result = TC_FAIL;
        }

        return result;
}
// code from tinycrypt end

/* This code attempt to solve the drm problem of song encryption and decryption */

/* structure for song protection */
//  struct mvpdata{
//  	uint8_t * enc_song[16];
//     uint8_t * master_pub_key[ECC_PUB_KEY_SIZE];
//  };


// struct user{
//     uint8_t priv_key[ECC_PRV_KEY_SIZE];
//     uint8_t pub_key[ECC_PUB_KEY_SIZE];
//     uint8_t username[15];
// };

// this struct is for reading the audio file
// WAVE file header format
struct HEADER {
	unsigned char riff[4];						// RIFF string
	unsigned int overall_size;				// overall size of file in bytes
	unsigned char wave[4];						// WAVE string
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	unsigned int format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;						// no.of channels
	unsigned int sample_rate;					// sampling rate (blocks per second)
	unsigned int byterate;						// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;					// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	unsigned int data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

//int encrypt_song (uint8_t* key, uint8_t* raw_song, uint8_t* enc_song);
int encrypt_song (uint8_t* key, uint8_t* raw_song, uint8_t* enc_song); // need to clean song.c since expected result is gone
//int return_public_key(uint8_t* public_key);
int decrypt_song(uint8_t* key, uint8_t* raw_song, uint8_t* enc_song);
int key_gen(uint8_t * pub_key, uint8_t * priv_key);

//int decrypt_song(const TCAesKeySched_t s, uint8_t* priv_key, uint8_t* public_key);

int keyshared(const uint8_t* private_key, const uint8_t* others_pub, uint8_t* sharedkey);
int write_mapping(char key, int val);
int read_mapping(char key);

#endif /* __SONG_H__ */