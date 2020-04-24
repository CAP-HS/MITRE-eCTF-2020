#include <stdio.h>
#include <math.h>

#include "ecdh.h"
#include <string.h>

//#include "xil_sprintf.h"
//#include <stdlib.h>

#include "sha256.h"
#include "constantsA.h"

//#include <inttypes.h>

#include "aes.h"
//#include "aes_decrypt.c"
//#include "aes_encrypt.c"

#define NUM_OF_NIST_KEYS 16

////PARSE FUNCTIONS------------------------------------------------

int charconvert(char in){

	//printf("%c", in);
	if (in == '0'){
		return 0;
	}
	else if (in == '1'){
		return 1;
	}
	else if (in == '2'){
		return 2;
	}
	else if (in == '3'){
		return 3;
	}
	else if (in == '4'){
		return 4;
	}
	else if (in == '5'){
		return 5;
	}
	else if (in == '6'){
		return 6;
	}
	else if (in == '7'){
		return 7;
	}
	else if (in == '8'){
		return 8;
	}
	else if (in == '9'){
		return 9;
	}
	else if (in == 'a'){
		return 10;
	}
	else if (in == 'b'){
		return 11;
	}
	else if (in == 'c'){
		return 12;
	}
	else if (in == 'd'){
		return 13;
	}
	else if (in == 'e'){
		return 14;
	}
	else if (in == 'f'){
		return 15;
	}
	else {
		return 0;
	}

}

int hex_to_dec(char *in1, char *in2){

	return charconvert(in1)*16 + charconvert(in2);

}

int parsepub(char *input, uint8_t *outpub)
{

	unsigned int k;
	for (k=0;k<ECC_PUB_KEY_SIZE*2;k+=2){

		int temp = hex_to_dec(input[k], input[k+1]);
		outpub[k/2] = temp;
	}


	return 0;

}


int importUsermap(char users[64][16], char pubs[64][100])
{
    	FILE *fp;
	int siz = ECC_PUB_KEY_SIZE*2+16+1;
    	char str[siz];
    	char* filename = "user_map.txt";

	char * tokenPtr;


	fp = fopen(filename, "r");
	int ind = 0;
	printf("\n");
	while (fgets(str, siz, fp) != NULL){

		tokenPtr = strtok (str, " ");
		char * tokens [100]; //Need a dynamic assignation in real time or exact value as required
		int count = 0;

		while(tokenPtr != NULL)
		{
			tokens[count] = tokenPtr; //tokens holds the strings you require
			count = count + 1;
			tokenPtr = strtok(NULL, " ");
		}

		strcpy(users[ind],tokens[0]);
		strcpy(pubs[ind],tokens[1]);

		ind = ind + 1;
	}

	/*unsigned int l;
	for (l = 0; l < 3; ++l){
		printf("%s\n", users[0]);
	}*/

	fclose(fp);
	return 0;
}

char* retrievePub(char *user)
{
	
	char users[64][16];
	char pubs[64][100];
	static char retrieved[100];

	importUsermap(users, pubs); //Thank GOD this works

	int found = 1;
	unsigned int p;
	int index = 0;
	for (p=0;p<64;++p){
		if (strcmp(user,users[p]) == 0){
			index = p;
			found = 0;
		}

	}
	if (found == 0){
		strcpy(retrieved,pubs[index]);
	}
	else{
		return NULL;

	}


	return retrieved;
}

int parsesongK(char *input, uint8_t *encsongKey)
{
	unsigned int k;
	for (k=0;k<16*2;k+=2){

		int temp = hex_to_dec(input[k], input[k+1]);
		encsongKey[k/2] = temp;
	}

	return 0;

}

int importSongmap(char songusers[64][16], char songencKeys[64][32], char* filename)
{
    FILE *fp;
	int siz = 50;
    char str[siz];

	char * tokenPtr;



	fp = fopen(filename, "r");
	int ind = 0;
	printf("\n");
	while (fgets(str, siz, fp) != NULL){
		tokenPtr = strtok (str, " ");
		char * tokens [50]; //Need a dynamic assignation in real time or exact value as required
		int count = 0;

		while(tokenPtr != NULL)
		{
			tokens[count] = tokenPtr; //tokens holds the strings you require
			count = count + 1;
			tokenPtr = strtok(NULL, " ");
		}

		strcpy(songusers[ind],tokens[0]);
		strcpy(songencKeys[ind],tokens[1]);

		ind = ind + 1;
	}


	fclose(fp);
	return 0;
}


char* retrieveEncKey(char *user, char* filename)
{
	
	char songusers[64][16];
	char songencKeys[64][32];
	static char retrievedKey[100];

	importSongmap(songusers, songencKeys, filename); //Thank GOD this works

	int found = 1;
	unsigned int p;
	int index = 0;
	for (p=0;p<64;++p){
		if (strcmp(user,songusers[p]) == 0){
			index = p;
			found = 0;
		}
		//printf("%s	", users[p]);
		//printf("%s\n", user);
	}
	if (found == 0){
		strcpy(retrievedKey,songencKeys[index]);
	}
	else{
		return NULL;

	}

	//printf("Retrieved User:	%s\n", users[1]);
	//printf("Retrieved Pub key:	%s\n", pubs[1]);

	return retrievedKey;
}


////------------------------------------------------------------------
//HASHING

char* sha256hash(char *input,int n)
{
	uint8_t digest[32];
	char* out[32];
	char out2[31];

	struct tc_sha256_state_struct s;
	unsigned int i;
	unsigned int j;

	
        for (j = 0; j < n; ++j) {
		
		(void)tc_sha256_init(&s);

		tc_sha256_update(&s,(const uint8_t *) input, strlen(input));

	
		(void)tc_sha256_final(digest, &s);

		//Print out digest

		memset(out2, 0, sizeof(out2));//Debug: cure to extra characters
		
		for (i = 0; i < sizeof(digest); ++i) {
			xil_sprintf(out, "%02x",digest[i]); //Debug
			strcat(out2, out);
		}
		input = out2;
		
	}
	return input;
}



