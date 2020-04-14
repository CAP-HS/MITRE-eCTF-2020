
//Task:
//Make test to take username and public key string and output to a file. Make sure the file can be read back into another c script.


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


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

    	FILE * fptr;
   	fptr = fopen("user_pins.txt", "w");


	for(j = 0; j < sizeof(testint); ++j){
		printf("%c	", out2[j]);
		printf("%c	", testint[j]);
		printf("%d\n", testint[j]);
		sprintf(out, "%c",testint[j]); //Debug
		strcat(out2, out);
		//
		

	}

	//I need username and out2 to be written to a text file.


	



}
