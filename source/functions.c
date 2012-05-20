#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

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

int get_n_mathy(int x){
	return (int)floor(log(x) / log(2)) + 1;
}


void addSpacesToString(char output[], int startingLength){
	int numberOfTimesToRun = startingLength / 4;
	
	int i;
	for (i = 0; i <= numberOfTimesToRun; i++)
	{
		int make_room_at = (4 * (i+1)) + i;
		int room_to_make = 1;
		
		memmove(
			output + make_room_at + room_to_make,
			output + make_room_at,
			42 - (make_room_at + room_to_make)   // was 15, now strlen(output) // this should be number of bytes to copy/move
		);
		
		output[make_room_at] = ' ';
	}
}

void addMissingBitsToMakeFour(char io[], int numberOfBitsToOutput){
	int lengthOfString = strlen(io);
	
	if (lengthOfString < numberOfBitsToOutput){
		int count = numberOfBitsToOutput - lengthOfString;
		
		memmove(
			io + 0 + count,
			io + 0,
			lengthOfString +3 - (0 + count)    // was 15 now it's lengthOfString+3
		);
		
		int i;
		for (i = 0; i < count; i++){
			io[i] = '0';
		}
	}
	
	io[numberOfBitsToOutput] = '\0';   // fixed a bug where when you hit 'e' it like... prints a funny looking q at the end of the string.  
}



void byte_to_binary(char buf[], int x)
{
	int n = get_n(x);
	int flat_int = n+1;
	
	int startingZ = (int)pow(2, flat_int) >> 1;     // take 2^MAX_VALUE and then bit shift it right once
	int numberOfBitsToOutput = (4 - (n % 4)) + n; //  (4 - (n % 4)) + n
	
    char b[flat_int];   // was 9 and was a static char...
    b[0] = '\0';

    int z;
    for (z = startingZ; z > 0; z >>= 1)              //   x & 256 checks if the 256 bit is flagged in x...
    {                                           //   then it shifts to the next smallest bit... 128
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
	
	addMissingBitsToMakeFour(b, numberOfBitsToOutput);
	addSpacesToString(b, numberOfBitsToOutput);
	
	strcpy(buf, b);
}




// converts -127 into 0, leaves 127 completely alone
int if_negative_be_zero(int x){
	if (x < 0){
		return 0;
	}
	return x;
}

// always returns a positive number or zero though...
int if_positive_be_zero(int x){
	if (x > 0){
		return 0;
	}
	return -x;
}


