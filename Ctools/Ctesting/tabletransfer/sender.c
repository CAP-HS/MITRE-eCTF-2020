
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


//Goal: Create a C table and export to file



//Set uint8_t buffers and username buffers
//Uint8_t buffers are all the same size
//Structure for row: username in bytes, public key in bytes

//Convert 

int main(){

	//username
	char username[16] = "Cream";
	printf("%s\n", username);

	unsigned int u;
	for (u = 0; u < sizeof(username); ++u){

		printf("%c", username[u]);

	}
	printf("\n");

	//mock public key
	char testchar[20] = "jsofjreoijhsifajlkdsjlkdsfjdsk";
	uint8_t testint[20];
	memcpy(testint, testchar, sizeof(testint));
	char out[20];
	char out2[20];
	memset(out2,0,sizeof(out2));
	unsigned int j;
	for(j = 0; j < sizeof(testint); ++j){
		printf("%c	", out2[j]);
		printf("%c	", testint[j]);
		printf("%d\n", testint[j]);
		sprintf(out, "%c",testint[j]); //Debug
		strcat(out2, out);
	}

	//printf("%s", out2);
	//printf("%s\n", username);
	//printf("%s\n", testint);

	//char endstring[20];
	//memset(endstring, 0, sizeof(endstring));
	//sprintf(endstring, "%c", testint);

	printf("Marker 1:	%s\n", out2);

	//Now to go from string buffer(out2) back to uint8_t testint.
	//This show that you can take a string, copy it to a uint_8 buffer 
	
	uint8_t gotit[20];
	memset(gotit, 0 , sizeof(gotit));
	memcpy(gotit, out2, sizeof(gotit));

	for(j = 0; j < sizeof(gotit); ++j){
		printf("%c	", gotit[j]);
		printf("%d\n", gotit[j]);

	}

	////START LOOP

		

		////This process may be slow
		//Make ecdh public keys

		//Turn ecdh public keys into strings

		//Concat username and string with space in between

		//Write new string to file


	return 0;

}
