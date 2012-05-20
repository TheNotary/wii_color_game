#include <string.h>
#include <stdio.h>
#include <math.h>

//#include <../include/functions.h>

// used by Byte_to_binary
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

int get_n_mathy(int x)
{
	
	return (int)floor(log(x) / log(2)) + 1;
}


void addSpacesToString(char output[], int startingLength)
{
	
}



void byte_to_binary(char buf[], int x)
{
	int MAX_VALUE = get_n(x)+1;
	
	int startingZ = (int)pow(2, MAX_VALUE) >> 1;     // take 2^MAX_VALUE and then bit shift it right once
	
	int numberOfBitsToOutput = (4 - (MAX_VALUE % 4)) + MAX_VALUE; //  (4 - (n % 4)) + n
	
	sprintf(buf, "%d", numberOfBitsToOutput);
	//printf(buf);
	
	
	
    char b[MAX_VALUE];   // was 9 and was a static char...
    b[0] = '\0';

    int z;
    for (z = startingZ; z > 0; z >>= 1)              //   x & 256 checks if the 256 bit is flagged in x...
    {                                           //   then it shifts to the next smallest bit... 128
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
	
	char prettyString[20];
	
	addSpacesToString(prettyString, numberOfBitsToOutput);

}



