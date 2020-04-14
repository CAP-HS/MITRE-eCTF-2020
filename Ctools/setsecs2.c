#include <stdio.h>


#include "ecdh.h"
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "constantsA.h"

#include <inttypes.h>
#include "aes_decrypt.h"
#include "aes_encrypt.h"

#define NUM_OF_NIST_KEYS 16



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
uint8_t* getKeys(char *input)
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
}

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

//This function is for createUser
char* getkeystring(char *input)
{
//This should take in a hash of private key for user and output a string for the public key
	//NOTE: Pin is a hash

	//printf("After: %s\n", input);
	//sha256hash(input,1);
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
	//printf("\n");
	input = out2;
	//printf("OUT: %s\n", input);
	//Return string form of public key

	//printf("%c\n", out2[0]);	
	parsepub(input);

	return input;
}

////PARSE FUNCTIONS-----------------------

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

int parsepub(char *input)
{
	static uint8_t outpub[ECC_PUB_KEY_SIZE];
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


int testimportUsermap()
{
    	FILE *fp;
	int siz = ECC_PUB_KEY_SIZE*2+16+1;
    	char str[siz];
    	char* filename = "user_map.txt";

	fp = fopen(filename, "r");
	while (fgets(str, siz, fp) != NULL){
        	printf("%s", str);
	}

	fclose(fp);
	return 0;
}



////------------------------------------------------------------------

/*struct User createUserObject(char* name[10], char* input, struct private_key_class priv, struct public_key_class pub, struct User user)
{
	//Use input for encryption
	//Create digest key and encrypt
	uint8_t digest[32];
	struct tc_sha256_state_struct s;	
	(void)tc_sha256_init(&s);
	tc_sha256_update(&s, input, strlen(input));
	(void)tc_sha256_final(digest, &s);
	
	struct tc_aes_key_sched_struct sk;
	uint8_t ciphertext[NUM_OF_NIST_KEYS];
	uint8_t private[16];
	memcpy(private, &priv, sizeof(priv));
	(void)tc_aes128_set_encrypt_key(&sk, digest);
	tc_aes_encrypt(ciphertext, private, &sk);
	//-------------------------

	//Create User Struct-------
	//struct User user;
	//memcpy(user.name,name,sizeof(name));
	printf("SHow name: %s\n",name);
	sprintf(user.name,name);
	user.publ = pub;
	memcpy(user.priv, ciphertext, sizeof(ciphertext));
	sprintf(user.passcheck, sha256hash(input, 20));

        printf("SHow name2: %s\n",user.name);
	
	//-------------------------
	
	return user;

}

void *createSecrets(char *hosts, char *pins, int num)
{
	//inputs are users and pins

	//char *host = "drew";
	//char *input = "01234677567";

	//printf("Size of host: %zu\n", sizeof(hosts[0][1]));
	//printf("%zu\n", sizeof(pins));
	
	unsigned int k;
	unsigned int characters = sizeof(hosts);//sizeof(hosts[0][0]);
	printf("Characters: %d\n",characters);
	printf("User: %c\n", hosts);
	printf("Pin: %s\n", pins);
	
	/*for (k = 0; k < characters; ++k){
		printf("Line %d : %s\n", k, hosts[0][k]);

	}
        //printf("Host: %p\n", hosts[1]=);
        
	
	struct public_key_class pub[1];
  	struct private_key_class priv[1];
	struct User user;
	//For each user do the following loop
	unsigned int j;	
	
        /*for (j = 0; j < num; ++j){ 
  	    rsa_gen_keys(pub, priv, PRIME_SOURCE_FILE);
            printf("%d\n",j);
	    user[j] = createUserObject(hosts[j], pins[j], priv[0], pub[0], user[j]);
	   /* for (k = 0; k < sizeof(user[j].name); ++k){
            	printf("WE are here: %s\n",user[j].name);
		printf("Hello 2 : %s\n",user[j].passcheck);
	    }

        //}

   	//Single User Implementation
	rsa_gen_keys(pub, priv, PRIME_SOURCE_FILE);
        user = createUserObject(hosts, pins, priv[0], pub[0], user);

	//return user;

}*/

/*int main(int argc, char **argv){

	printf("Testing");





	return 0;
}*/

