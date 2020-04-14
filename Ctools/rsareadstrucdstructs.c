#include <stdio.h>
#include "rsa.h"
#include <string.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
	//Takes in keys and reads parameters
	FILE *infile; 
	FILE *infile2;
	struct public_key_class pub[1];
	struct private_key_class priv[1]; 

		
	infile = fopen ("pub.dat", "r");
	infile2 = fopen ("priv.dat", "r");
	
	fread(&pub, sizeof(struct public_key_class), 1, infile);
	fread(&priv, sizeof(struct private_key_class), 1, infile2);

	  printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)priv->modulus, (long long) priv->exponent);
  	printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)pub->modulus, (long long) pub->exponent);

	fclose(infile);
	fclose(infile2);


}
