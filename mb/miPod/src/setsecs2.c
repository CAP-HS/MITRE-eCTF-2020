#include <stdio.h>
#include <math.h>

#include "ecdh.h"
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "constantsA.h"

#include <inttypes.h>

#include "aes.h"

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
	//static uint8_t outpub[ECC_PUB_KEY_SIZE];
	unsigned int k;
	printf("Parse Pub: \n");
	//char temp;
	for (k=0;k<ECC_PUB_KEY_SIZE*2;k+=2){

		int temp = hex_to_dec(input[k], input[k+1]);
		outpub[k/2] = temp;
	}
	printf("\n");


	for (k=0;k<ECC_PUB_KEY_SIZE;++k){
		printf("%d ", outpub[k]);
	}
	printf("\nParse Complete \n");

	return 0;

}


int importUsermap(char users[64][16], char pubs[64][100])
{
    	FILE *fp;
	int siz = ECC_PUB_KEY_SIZE*2+16+1;
    	char str[siz];
    	char* filename = "user_map.txt";

	char * tokenPtr;

	//char users[64][16];
	//char pubs[64][100];


	fp = fopen(filename, "r");
	int ind = 0;
	printf("\n");
	while (fgets(str, siz, fp) != NULL){
        	//printf("%s", str);
		tokenPtr = strtok (str, " ");
		char * tokens [100]; //Need a dynamic assignation in real time or exact value as required
		int count = 0;

		while(tokenPtr != NULL)
		{
			tokens[count] = tokenPtr; //tokens holds the strings you require
			count = count + 1;
			tokenPtr = strtok(NULL, " ");
		}
		//sprintf(temp,"%s", tokens[0]);
		//sprintf(temp2,"%s", tokens[1]);

		//printf("%s	%d\n", temp,ind);
		//printf("%d\n", ind);
		//temp[ind] = tokens[0];
		strcpy(users[ind],tokens[0]);
		strcpy(pubs[ind],tokens[1]);
		//printf("%s	%s\n",users[0], users[ind]);

		//memset(tokens, 0, 100);
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
			printf("Found index: %d\n	", p);
			index = p;
			found = 0;
		}
		//printf("%s	", users[p]);
		//printf("%s\n", user);
	}
	if (found == 0){
		strcpy(retrieved,pubs[index]);
		printf("%s\n", retrieved);
	}
	else{
		return NULL;

	}

	//printf("Retrieved User:	%s\n", users[1]);
	//printf("Retrieved Pub key:	%s\n", pubs[1]);

	return retrieved;
}

int parsesongK(char *input, uint8_t *encsongKey)
{
	//static uint8_t outpub[ECC_PUB_KEY_SIZE];
	unsigned int k;
	printf("Parse Pub: \n");
	//char temp;
	for (k=0;k<16*2;k+=2){

		int temp = hex_to_dec(input[k], input[k+1]);
		encsongKey[k/2] = temp;
	}
	printf("\n");


	for (k=0;k<16;++k){
		printf("%d ", encsongKey[k]);
	}
	printf("\nParse Complete \n");

	return 0;

}

int importSongmap(char songusers[64][16], char songencKeys[64][32], char* filename)
{
    	FILE *fp;
	int siz = 50;
    	char str[siz];
    	//char* filename = "user_map.txt";

	char * tokenPtr;

	//char users[64][16];
	//char pubs[64][100];


	fp = fopen(filename, "r");
	int ind = 0;
	printf("\n");
	while (fgets(str, siz, fp) != NULL){
        	//printf("%s", str);
		tokenPtr = strtok (str, " ");
		char * tokens [50]; //Need a dynamic assignation in real time or exact value as required
		int count = 0;

		while(tokenPtr != NULL)
		{
			tokens[count] = tokenPtr; //tokens holds the strings you require
			count = count + 1;
			tokenPtr = strtok(NULL, " ");
		}
		//sprintf(temp,"%s", tokens[0]);
		//sprintf(temp2,"%s", tokens[1]);

		//printf("%s	%d\n", temp,ind);
		//printf("%d\n", ind);
		//temp[ind] = tokens[0];
		strcpy(songusers[ind],tokens[0]);
		strcpy(songencKeys[ind],tokens[1]);
		//printf("%s	%s\n",songusers[0], songusers[ind]);

		//memset(tokens, 0, 50);
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
			printf("Found index: %d\n	", p);
			index = p;
			found = 0;
		}
		//printf("%s	", users[p]);
		//printf("%s\n", user);
	}
	if (found == 0){
		strcpy(retrievedKey,songencKeys[index]);
		printf("%s\n", retrievedKey);
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
	char* out3[31];
	struct tc_sha256_state_struct s;
	unsigned int i;
	unsigned int j;

	printf("Input Received: %s\n", input);
	
        for (j = 0; j < n; ++j) {
		
		(void)tc_sha256_init(&s);

		tc_sha256_update(&s,(const uint8_t *) input, strlen(input));

	
		(void)tc_sha256_final(digest, &s);

		//Print out digest

		memset(out2, 0, sizeof(out2));//Debug: cure to extra characters
		
		for (i = 0; i < sizeof(digest); ++i) {
			sprintf(out, "%02x",digest[i]); //Debug
			strcat(out2, out);
		}
		input = out2;
		
	}
	printf("Digest: %s\n", input);
	return input;
}

//struct User *getKeys(char *input)
/*uint8_t* getKeys(char *input)
{
	printf("Key sizes: pub %d / prv %d\n", ECC_PUB_KEY_SIZE, ECC_PRV_KEY_SIZE);
	static uint8_t pub[ECC_PUB_KEY_SIZE];
  	static uint8_t prv[ECC_PRV_KEY_SIZE];
	//struct public_key_class pub[1];
  	//struct private_key_class priv[1];
	//struct User *user;
	
	//user = malloc(sizeof(struct User));
	
	memcpy(prv, input, ECC_PRV_KEY_SIZE);
	unsigned int h;
	printf("Prv with input: \n");
	for (h = 0; h <sizeof(prv); ++h) {
		printf("%02x", prv[h]);

	}
	printf("\n");

	printf("Generating Keys (0 is for fail, 1 is for pass)...\n");
	printf("Generation Success? :  %d\n",ecdh_generate_keys(pub, prv));

	return pub;
}*/

//This function will take in the user private and master private keys, create a shared secret and use the shared secret to encrypt the master private key.
/*uint8_t* createSharedSecretKeyANDencrypt(char* input, char* MasterPriv)
{
	//Setup user public key
	static uint8_t pub[ECC_PUB_KEY_SIZE];
  	static uint8_t prv[ECC_PRV_KEY_SIZE];
	memcpy(prv, input, ECC_PRV_KEY_SIZE);
	ecdh_generate_keys(pub, prv);

	//Setup song key
	uint8_t digest[32];
	struct tc_sha256_state_struct s;	
	(void)tc_sha256_init(&s);
	tc_sha256_update(&s, MasterPriv, strlen(MasterPriv));
	(void)tc_sha256_final(digest, &s);
	//static uint8_t songpub[ECC_PUB_KEY_SIZE];
  	static uint8_t songprv[ECC_PRV_KEY_SIZE];
	memcpy(songprv, digest, ECC_PRV_KEY_SIZE);
	//ecdh_generate_keys(songpub, songprv);	

	//Now the shared secret can be made using the public keys
	static uint8_t shared[ECC_PUB_KEY_SIZE];	
	ecdh_shared_secret(priv, Master public_____________, shared);

	//Use shared key to encrypt master priv key (songprv)
  	/*static uint8_t encsongprv[ECC_PRV_KEY_SIZE];	
	struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_encrypt_key(&sk, shared);
	tc_aes_encrypt(encsongprv, songprv, &sk);

	return encsongprv; //This could also go straight into a map inside this function
}*/

////Public key generation------------------------------------------------------

//This function is for createUser
char* getkeystring(char *input)
{
//This should take in a hash of private key for user and output a string for the public key
	//NOTE: Pin is a hash

	//Generate ecdh pub key	
	static uint8_t pub[ECC_PUB_KEY_SIZE];
  	static uint8_t prv[ECC_PRV_KEY_SIZE];
	memcpy(prv, input, ECC_PRV_KEY_SIZE);
	ecdh_generate_keys(pub, prv);

	//printf("Pub string: %s\n", pub);

	//Create output string buffer
	char out[ECC_PUB_KEY_SIZE];
	char out2[ECC_PUB_KEY_SIZE*2];
	memset(out2,0,sizeof(out2));
	unsigned int j;
	for(j = 0; j < sizeof(pub); ++j){
		printf("%d ", pub[j]);
		//printf("%c	", testint[j]);
		//printf("%d\n", testint[j]);
		sprintf(out, "%02x",pub[j]); //Debug
		strcat(out2, out);
	}

	input = out2;
	return input;
}


////AES encryption-------------------------------------------------------------


//Encrypt song key with shared secret (This will be run from protectSong)
//Needs: user priv, master pub and song key
//Output: string of encrypted song key
char* encsongkey(char *userpriv,char *masterpub,char *songkey)
{
	//Setup user
  	static uint8_t prv[ECC_PRV_KEY_SIZE];
	memcpy(prv, userpriv, ECC_PRV_KEY_SIZE);
	//Setup master pub
	static uint8_t outpub[ECC_PUB_KEY_SIZE];
	parsepub(masterpub, outpub);
	//Setup songkey
	static uint8_t songK[16]; //Determined song key size
	memcpy(songK, songkey, sizeof(songK));

	//Create shared secret
	static uint8_t shared[ECC_PUB_KEY_SIZE];	
	ecdh_shared_secret(prv, masterpub, shared);
	
	//encrypt songkey
 	static uint8_t encsongK[16];	
	struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_encrypt_key(&sk, shared);
	tc_aes_encrypt(encsongK, songK, &sk);

	//Output key string
	char out[16];
	static char out2[16*2];
	memset(out2,0,sizeof(out2));
	unsigned int j;
	for(j = 0; j < sizeof(encsongK); ++j){
		//printf("%d ", encsongK[j]);
		//printf("%c	", testint[j]);
		//printf("%d\n", testint[j]);
		sprintf(out, "%02x",encsongK[j]);
		strcat(out2, out);
	}

	//printf("Encrypted song key inside: %s", out2);
	//Return string
	return out2;
}

int enc_song(uint8_t * fullsong, char* key, int songlen, uint8_t encode[songlen])
{//Edmund
	//static int lengtmp = songlen;
	printf("songlen: %d\n",songlen);
	//printf("Length: %d", ceil(songlen/16)*16);
	uint8_t padsong[songlen];//ceil(songlen/16)*16];
	memset(padsong,0,sizeof(padsong));
	memcpy(padsong,fullsong,sizeof(padsong));

	//int16_t tempenc[songlen];

	
	uint8_t tmp[16];
	uint8_t tmp2[16];
	size_t read = 0;
	size_t write; 
	int k=0,i = 0;
	//uint8_t * p = encode;
	uint8_t *padptr = padsong;

	static uint8_t songK[16]; //Determined song key size
	memcpy(songK, key, sizeof(songK));
	struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_encrypt_key(&sk, songK);

	
	for(k = 0; k<sizeof(padsong);++k){
		if (k < 16){
			printf("%d ", padsong[k]);
		}
	};
	printf("\n");
	printf("Debug 1\n");
	//int16_t *track = encode;
	int offset = 0;
	for(int j = 0; j < songlen/16;++j)
	{
		if( j + 1 == songlen/16){	//If this is the last loop
			for (k = 0; k < 16; k++){
				tmp[k] = padptr;
				padptr = padptr + 1;
			}
			tc_aes_encrypt(encode, tmp, &sk);
			//memcpy(track, tmp2, 16);
			//printf("Last\n");
		}else{	//This is not the last loop
			for (k = 0; k < 16; k++){
				tmp[k] = padptr;
				//if( j < 16){
				//	printf("%02x ",tmp[k]);
				//}
				padptr = padptr + 1;
			}
			tc_aes_encrypt(encode+offset, tmp, &sk);
			//memcpy(track, tmp2, 16);
			//printf("%d	", encode[j]);
			//printf("Ite: %d\n", j);
			offset+=16;
  
		}
		//memset(tmp,0,sizeof(tmp));
		//memset(tmp2,0,sizeof(tmp2));		
		
	}
	//encode-=songlen;
	printf("\n");
	for (int j = 0; j < songlen; ++j){
		if (j<16){
			printf("%d ", encode[j]);
		}
	}

	//printf("%d\n", encode[0]);
	//printf("%d\n", encode[1]);
	printf("Done\n");
	//free(track);
	//free(p);
	return 0;
}





