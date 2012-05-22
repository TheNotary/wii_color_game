#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gccore.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif





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



double convertJoyToDegrees(int joy_x, int joy_y){
	int delta = 5;
	if ( (joy_x < delta && joy_x > -delta) && (joy_y < delta && joy_y > -delta) ){   // if we're in the deadzone...
		return -1.0;
	}
	
	double degrees = atan2(joy_x, joy_y) * 180 / M_PI;             // convert joystick input to degrees
	
	if (degrees < 0){ // correct negatives as parts of the whole 360 degree spectrum...
		degrees = 360 + degrees;
	}
	return degrees;
}




// params are...
//   starting colors(3)
//   Starting offset, regionSize
//   Color Deltas for region (3)
//   degrees of joystick position.
GXColor calculateColorForRegion(int red, int green, int blue,
									int startingOffset, int regionSize,
									int redDif, int greenDif, int blueDif, int degrees){
	GXColor bg;
	//red = 0x00;
	//green = 0x00;
	//blue = 0xff;
	
	//startingOffset = 0;
	double degreesThroughRegion = degrees - startingOffset;
	double percentageThroughRegion = degreesThroughRegion / regionSize;
	/*
	redDif = 0;
	greenDif = 255;
	blueDif = -127;
	*/
	red   += redDif   * percentageThroughRegion;
	green += greenDif * percentageThroughRegion;
	blue  += blueDif  * percentageThroughRegion;
	
	bg = (GXColor){red,green,blue,0xff};
	return bg;
}




// the color wheel has 12 base colors, and this code allows the joystick to be mapped to 
// some gradient between those colors or one of those base colors themselves
GXColor setBackgroundBasedOnDegrees(GXColor background, double degrees){
	static GXColor bg;
	
	// From solid blue to a blue green... 30 degrees... special case
	// base is (0,0,ff) -> (0,ff,7f)
	// refer to:  http://processingjs.org/learning/basic/colorwheel/
	bool isRegion1 = (degrees >= 0 && degrees < 30);
	bool isRegion2 = (degrees >= 30 && degrees < 60);   // blue-green to green 30deg
	bool isRegion3 = (degrees >= 60 && degrees < 120);  // green to yellow 60deg
	bool isRegion4 = (degrees >= 120 && degrees < 240); // from yellow all the way to red... in picture
	bool isRegion5 = (degrees >= 240 && degrees < 300);
	bool isRegion6 = (degrees >= 300 && degrees < 360);
	
	if (isRegion1){
		bg = calculateColorForRegion(0,0,255,0,30,0,255,-127,degrees);
	}
	else if (isRegion2){
		bg = calculateColorForRegion(0,255,127,30,30,0,0,-127,degrees);
	}
	else if (isRegion3){
		bg = calculateColorForRegion(0,255,0,60,60,255,0,0,degrees);
	}
	else if (isRegion4){
		bg = calculateColorForRegion(0xff, 0xff, 0, 120, 120, 0, -255, 0, degrees); 
	}
	else if (isRegion5){
		bg = calculateColorForRegion(255,0,0,240,60,0,0,255,degrees);
	}
	else if (isRegion6){
		bg = calculateColorForRegion(255,0,255,300,60, -255,0,0,degrees);
	}
	
	background = bg;
	return bg;
}


int abs(int x){
	if(x < 0){
		return -x;
	}
	return x;
}


bool deadZoneClearance(int joy_x, int joy_y, int old_x, int old_y)
{
	int delta = 15;
	if (abs(old_x - joy_x) > delta || abs(old_y - joy_y) > delta){ 
		return true;
	}
	
	return false;
}