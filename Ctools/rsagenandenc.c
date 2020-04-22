#include <stdio.h>
#include "rsa.h"
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "constantsA.h"

#include <inttypes.h>
#include "aes_decrypt.h"
#include "aes_encrypt.h"

#define NUM_OF_NIST_KEYS 16


struct User {
	public_key_class publ;
	uint8_t priv[16];
	char* passcheck[31];
	
};


//function to convert ascii char[] to hex-string (char[])
void string2hexString(char* input, char* output)
{
    int loop;
    int i; 
    
    i=0;
    loop=0;
    
    while(input[loop] != '\0')
    {
        sprintf((char*)(output+i),"%02X", input[loop]);
        loop+=1;
        i+=2;
    }
    //insert NULL at the end of the output string
    output[i++] = '\0';
}
char *sha256hash(char* input,int n)
{
	uint8_t digest[32];
	char* out[32];
	char out2[31];
	char* out3[31];
	struct tc_sha256_state_struct s;
	unsigned int i;
	unsigned int j;

	//printf("%s\n",input); //Debug
        for (j = 0; j < n; ++j) {
		
		(void)tc_sha256_init(&s);
		/*printf("IV2: %x %x %x %x %x %x %x %x\n", s.iv[0],s.iv[1],s.iv[2],s.iv[3],s.iv[4],s.iv[5],s.iv[6],s.iv[7] ); //Debug*/

		tc_sha256_update(&s, /*(const uint8_t *)*/ input, strlen(input));

	
		(void)tc_sha256_final(digest, &s);

		//Print out digest

		memset(out2, 0, sizeof(out2));//Debug: cure to extra characters

		//digest_1 := strconv.itoa(digest);//Debug
		
		for (i = 0; i < sizeof(digest); ++i) {
			//printf("%02x",digest[i]); //Debug
			//char* h = (char*) digest[i];
			//printf("%s\n", out2);
			sprintf(out, "%02x",digest[i]); //Debug
			//printf("%s\n",out2);
			strncat(out2, out);
			//printf("%02x\n",h);
		}
		//Debug: For loop is to carry out 3 iterations of calling function sha256_1
		//printf("%s\n",out2);
		input = out2;
		//output_file = out2;//Debug to cure function transfer
		
		//return out2;
	}
	//out3 = out2;
	return input;
}


User createUserObject(char* input, struct private_key_class priv, struct public_key_class pub)
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
	struct User user;
	user->publ = pub;
	user->priv = ciphertext;
	sprintf(user.passcheck, sha256hash(input, 20));
	
	//-------------------------
	
	return user;

}

uint8_t *sha256enc(char* input, struct private_key_class priv, struct public_key_class pub)
{

	
	
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
	struct User user;
	user.publ = pub;
	user.private = ciphertext;
	sprintf(user.passcheck, sha256hash(pin, 20));
	
	//-------------------------
	

	//Decryption---------------
	//uint8_t digest[32];
	//struct tc_sha256_state_struct s;	
	(void)tc_sha256_init(&s);
	tc_sha256_update(&s, input, strlen(input));
	(void)tc_sha256_final(digest, &s);
	//struct tc_aes_key_sched_struct sk;
	(void)tc_aes128_set_decrypt_key(&sk, digest);

	uint8_t decrypted[NUM_OF_NIST_KEYS];
	tc_aes_decrypt(decrypted, ciphertext, &sk); //Ciphertext comes from user struct
	struct private_key_class r;
	memcpy(&r,decrypted,sizeof(decrypted));
	printf("Dec_struct: %lld\n", r.modulus); //r is user priv
	//-------------------------	




	return digest;
	
}


int main(int argc, char **argv)
{
	//Testing	
	char *pin = "abc";

	char output_value[32];
	char* passcheck[31];
	char* encuser[31];

	int len = strlen(pin);
	char hex_str[(len*2)+1];
	string2hexString(pin, hex_str);
	
	printf("ascii_str: %s\n", pin);
   	//printf("hex_str: %s\n", hex_str);

	struct public_key_class pub[1];
  	struct private_key_class priv[1];
  	rsa_gen_keys(pub, priv, PRIME_SOURCE_FILE);
	  printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)priv->modulus, (long long) priv->exponent);
	  printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)pub->modulus, (long long) pub->exponent);
	/*printf("%zu\n",sizeof(pub));
	printf("%zu\n",sizeof(priv));*/
	printf("First priv: %lld\n",priv->modulus);
	
	// Take in and alter pin
	sprintf(encuser, sha256hash(pin, 1));
	sprintf(passcheck, sha256hash(pin, 20));
	printf("%s\n", encuser);
	printf("%s\n", passcheck);

	sha256enc(pin, priv[0], pub[0]);
	
	// AES encrypt struct
	

	// AES decrypt struct for test
	//aesdec();

	// Put the structs in array
	

	FILE *outfile; 
	FILE *outfile2;
      
	// open file for writing 
	outfile = fopen ("pub.dat", "w");
	outfile2 = fopen("priv.dat", "w");


	//Write structures to files
	fwrite (&pub, sizeof(struct public_key_class), 1, outfile);
 
	fwrite (&priv, sizeof(struct private_key_class), 1, outfile2);


	fclose (outfile);
	fclose (outfile2);


} 
