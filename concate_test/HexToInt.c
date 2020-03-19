#include <stdio.h>
#include "inttypes.h"
#include "stdint.h"
#include <string.h>


//Ignore majority of this code 


int getNum(char ch)
{
    int num=0;
    if(ch>='0' && ch<='9')
    {
        num=ch-0x30;
    }
    else
    {
        switch(ch)
        {
            case 'A': case 'a': num=10; break;
            case 'B': case 'b': num=11; break;
            case 'C': case 'c': num=12; break;
            case 'D': case 'd': num=13; break;
            case 'E': case 'e': num=14; break;
            case 'F': case 'f': num=15; break;
            default: num=0;
        }
    }
    return num;
}

//function : hex2int
//this function will return integer value against
//hexValue - which is in string format
 
unsigned int hex2int(unsigned char hex[])
{
    unsigned int x=0;
    x=(getNum(hex[0]))*16+(getNum(hex[1]));
}
int main()
{
    uint8_t test[4] = {0x54, 0x1a, 0x45, 0x2b};
    uint8_t hexValue[2]= "9A";
    unsigned int  intValue=0; //can be stored in unsigned char
    unsigned char sr1 [20];
    int a;
    intValue=hex2int(hexValue);
     
    printf("value is: %d\n",intValue);
    printf("this works \n");
   // printf("%d \n", (int) test);
    for (int i = 0; i < 4; i++ )
    {
        a = (int) test[i];
        printf("%d \n", a);
        sprintf(sr1, "%d", a);
      
        //This is where I am having a problem with this concatenation
        strcat(sr1, sr1);
    }
    
    printf("%s \n", sr1);
    return 0;


}