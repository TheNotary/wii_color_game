#include <string.h>
#include <stdio.h>
#include <math.h>

#include <../include/functions.h>

// this get's n, in the equation 2^n = integer
int get_n(int x){    // assume 16
	int n = 0;
	
	int z;
	for (z = x; z > 0; z >>= 1)
	{
		n++;
	}
	
	return n-1;
}


const char *byte_to_binary(int x)
{
	char buf[5];
	sprintf(buf, "%d", x);
	printf(buf);
	
	// int MAX_VALUE = length_of_x(x);   //  (4 - (n % 4)) + n
	int MAX_VALUE = get_n(x)+1;
	
	int startingZ = (int)pow(2, MAX_VALUE) >> 1;     // take 2^MAX_VALUE and then bit shift it right once
	
	sprintf(buf, "%d", MAX_VALUE);
	printf(buf);
	sprintf(buf, "%d", startingZ);
	printf(buf);
	
	
    char b[MAX_VALUE];   // was 9 and was a static char...
    b[0] = '\0';

    int z;
    for (z = startingZ; z > 0; z >>= 1)              //   x & 256 checks if the 256 bit is flagged in x...
    {                                           //   then it shifts to the next smallest bit... 128
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}



