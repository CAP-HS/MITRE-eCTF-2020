#include <stdio.h>
#include <math.h>

#include "ecdh.h"
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "constantsA.h"

#include <inttypes.h>
#include "aes_decrypt.h"
#include "aes_encrypt.h"

#define NUM_OF_NIST_KEYS 16

////PARSE FUNCTIONS------------------------------------------------

int charconvert(char in);

int hex_to_dec(char *in1, char *in2);

int parsepub(char *input, uint8_t *outpub);


int importUsermap(char users[64][16], char pubs[64][100]);

char* retrievePub(char *user);

int parsesongK(char *input, uint8_t *encsongKey);

int importSongmap(char songusers[64][16], char songencKeys[64][32], char* filename);


char* retrieveEncKey(char *user, char* filename);



////HASHING-------------------------------------------------------------

char* sha256hash(char *input,int n);

////Public key generation------------------------------------------------------

char* getkeystring(char *input);


////AES encryption-------------------------------------------------------------

char* encsongkey(char *userpriv,char *masterpub,char *songkey);

int enc_song(uint8_t * fullsong, char* key, int songlen, uint8_t encode[songlen]);

