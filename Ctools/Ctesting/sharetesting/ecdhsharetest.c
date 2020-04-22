#include <stdio.h>


#include "ecdh.h"
#include <string.h>
#include <stdlib.h>


int main ()
{

	char input[] = "c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646";
	char input2[] = "7e071fd9b023ed8f18458a73613a0834f6220bd5cc50357ba3493c6040a9ea8c";

	//Generate ecdh pub keys	
	static uint8_t pub[ECC_PUB_KEY_SIZE];
  	static uint8_t prv[ECC_PRV_KEY_SIZE];
	memcpy(prv, input, ECC_PRV_KEY_SIZE);
	ecdh_generate_keys(pub, prv);

	static uint8_t pub2[ECC_PUB_KEY_SIZE];
  	static uint8_t prv2[ECC_PRV_KEY_SIZE];
	memcpy(prv2, input2, ECC_PRV_KEY_SIZE);
	ecdh_generate_keys(pub2, prv2);

	//Start ecdh shared secret section (Edmund has the rest)





	return 0;

}
