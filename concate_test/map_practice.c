#include "stdint.h"
#include "map.h"
#include "stdlib.h"
#include <stdio.h>
#include "inttypes.h"

#define maxSize 10
typedef map_t(unsigned int *) uint_map_t;

char * hex_to_char(uint8_t * hex)
{
    char * buffer2;
    int i;
    buffer2 = malloc(9);
    if(!buffer2)
    {
        return NULL;
    }

    buffer2[8] = 0;
    for (i = 0; i <= 7; i++)
    {
        buffer2[7-i] = (((*a) >> i) & (0x01)) + '0';
    }

    puts(buffer2);
    return buffer2;
}

// int hex_to_ascii(uint8_t c[], char d)
// {
//     int high = hex_to_int(c) * 16;
//     int low = hex_to_int(d);
//     return high+low;
// }

int main()
{
    uint_map_t mapsong;
    map_init(&mapsong);
    char title [] = "demo";
    const uint8_t data[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    char m[maxSize] = {0x0};
    int i = 0;

    char * test;
    test = 

    //map_set(&mapsong, title, data[i]);
    // printf("test 1 \n");
    // for (i = 0; i<10; i++)
    // {
    //     printf("test 2\n");
    //     strncat(m, data[i], maxSize - strlen(m)-1);
    //     printf("test 3\n");
    //     //map_set(&mapsong, title, data[i]);
    //     //uint8_t *val = map_get(&mapsong, title);
    //     //printf("%d \n", *val);
    // }
    // printf("%d \n", m);
    return 0;
}

/*int mp_data(char * rawsong, uint8_t  song_data)
{

   // map_init(&song_map);

    map_set(&song_map, rawsong, song_data);

    uint8_t *val = map_get(&song_map, rawsong);
    if (val){
        printf("value: %x\n", *val);
        printf(rawsong, "\n");
    } else {
        printf("value not found\n");
    }
    
    map_deinit(&song_map);
}*/



