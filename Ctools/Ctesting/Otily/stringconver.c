int main()
{ 


int n = 3;

  uint8_t user_pin [n] = {0x42, 0x21, 0x27};//The size of this array is the size of the pin and should be universal valued
   char user_str [n];
   user_str[n] = '\0';

   ConvertToString(user_pin, user_str);

}
char ConvertToString(uint8_t val[],char * user_str)
{
    uint8_t * ptr= &val[0];
    int n = 3; //n would be the number of terms in user_pin a fixed value
    for (int i = 0; i < n; i++)
    {
        user_str[i] = (char* ) *(ptr + (i * sizeof(val[0])));
    //This takes advantage of consecutive values of an array
    }
    printf("This is the uint8_t as a string: %s\n", user_str);
}
