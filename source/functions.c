#include <string.h>
#include <stdio.h>
#include <wiiuse/wpad.h>

#include <math.h>
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )


	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

#include <string.h>

#include <gccore.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef HALF_PI
#define HALF_PI 1.570796326794897
#endif

#ifndef DISTANCE_CAP
#define DISTANCE_CAP 85
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
//   Color Deltas for region's sweep (3)
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
	
	red   += redDif   * sin(percentageThroughRegion*HALF_PI); // this new equations makes it so it's easier to be "stuck" on one of the prime 8 colors...
	green += greenDif * sin(percentageThroughRegion*HALF_PI); // degrees should be adjusted counter clockwise by 1 or 2 to make it feel more centered...
	blue  += blueDif  * sin(percentageThroughRegion*HALF_PI); // Or a more clever algorithm should be implimented here...
	
	
	bg = (GXColor){red,green,blue,0xff};
	return bg;
}


double rotateDegreesBy(double degrees, double rotation){
	double result = degrees + rotation;
	if (result > 360.0){
		return result - 360.0;
	}
	else{
		return result;
	}
}



// the color wheel has 12 base colors, and this code allows the joystick to be mapped to 
// some gradient between those colors or one of those base colors themselves
GXColor setBackgroundBasedOnDegrees(GXColor background, double degrees){
	// Rotate degrees so yellow is the color on top (most intuitive)
	//degrees = rotateDegreesBy(degrees, 120.0);
	static GXColor bg;
	
	// From solid blue to a blue green... 30 degrees... special case
	// base is (0,0,ff) -> (0,ff,7f)
	// refer to:  http://processingjs.org/learning/basic/colorwheel/
	bool isRegion1 = (degrees >= 0   && degrees < 45);
	bool isRegion2 = (degrees >= 45  && degrees < 90);   // blue-green to green 30deg
	bool isRegion3 = (degrees >= 90  && degrees < 135);  // green to yellow 60deg
	bool isRegion4 = (degrees >= 135 && degrees < 180); // from yellow all the way to red... in picture
	bool isRegion5 = (degrees >= 180 && degrees < 225); // pink
	bool isRegion6 = (degrees >= 225 && degrees < 270); // this micro region is to emphasize the color purple... it's hacked in post v0.0
	bool isRegion7 = (degrees >= 270 && degrees < 315);
	bool isRegion8 = (degrees >= 315 && degrees < 360);
	// at 270 we want pink
	// at 315 we want purple
	
	
	if (isRegion1){  // blue
		bg = calculateColorForRegion(0,0,255,     0,45,     0,225,-255,   degrees);
	}
	else if (isRegion2){ // green
		bg = calculateColorForRegion(0,225,0,     45,45,    255,0,0,    degrees);
	}
	else if (isRegion3){ // yellow
		bg = calculateColorForRegion(255,255,0,   90,45,    0,-128,0,   degrees); 
	}
	else if (isRegion4){ // orange
		bg = calculateColorForRegion(255,127,0,   135,45,   0,-127,0,   degrees);
	}
	else if (isRegion5){ // red
		bg = calculateColorForRegion(255,0,0,     180,45,   0,165,153,    degrees);
	}
	else if (isRegion6){ // pink-sammon
		bg = calculateColorForRegion(255,165,153,   225,45, 0,0,102,    degrees);
	}
	else if (isRegion7){ // pink
		bg = calculateColorForRegion(255,165,255,   270,45, -95,-153,-55,    degrees);
	}
	else if (isRegion8){ // purple
		bg = calculateColorForRegion(160,0,200,   315,45, -160,0,55,    degrees);
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

double abs_d(double x){
	if(x < 0.0){
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



double getDistanceOfJoystickFromCenter(joystick_t joy_data){
	double result = sqrt(pow(joy_data.pos.x-joy_data.center.x, 2) + pow(joy_data.pos.y-joy_data.center.y, 2));
	if (result > DISTANCE_CAP){
		return DISTANCE_CAP;
	}
	return result;
}



double getDistanceOfJoystickFromOrigin(int joy_x, int joy_y){
	double result = sqrt(pow(joy_x, 2) + pow(joy_y, 2));
	if (result > DISTANCE_CAP){
		return DISTANCE_CAP;
	}
	return result;
}


GXColor darkenBackgroundBasedOnDistance(GXColor background, int joy_x, int joy_y){
	double dist = getDistanceOfJoystickFromOrigin(joy_x, joy_y);
	double darknessMult = abs_d((dist/DISTANCE_CAP));
	
	
	//printf(" ");
	//int calc = (int)(background.r * darknessMult);
	//printf("Joy:  %d, %d  ", joy_x, joy_y);
	//printf("dinstance:  %f   ", dist);
	//printf("Darkness Mult:  %f   ", darknessMult);
	//int count = 60;
	//while(count--) VIDEO_WaitVSync();
	
	
	background = (GXColor){(int)(background.r * darknessMult), 
							(int)(background.g * darknessMult), 
							(int)(background.b * darknessMult), 
							0xff};
	return background;
}



bool inDeadzone(joystick_t joy_data){
	if (abs(joy_data.pos.x - joy_data.center.x) < 10 && abs(joy_data.pos.y - joy_data.center.y) < 10)
		return true;
	return false;
}



bool joyMovementNegligable(int joy_x, int joy_y, int old_x, int old_y, int tolerance){
	double distance = sqrt(pow(joy_x-old_x, 2) + pow(joy_y-old_y, 2));
	if (distance < tolerance){
		return true;
	}
	else{
		return false;
	}
}









int countUpToSixty(int x){
	x++;
	if (x >59){
		return 0;
	}
	return x;
}




/*

//---------------------------------------------------------------------------------
// Texture co-ordinates for ball sprites
//---------------------------------------------------------------------------------
float texCoords[] = {
//---------------------------------------------------------------------------------
	0.0 ,0.0 , 0.5, 0.0, 0.5, 0.5, 0.0, 0.5,  // BALL 1
	0.5 ,0.0 , 1.0, 0.0, 1.0, 0.5, 0.5, 0.5,  // BALL 2
	0.0 ,0.5 , 0.5, 0.5, 0.5, 1.0, 0.0, 1.0,  // BALL 3
	0.5 ,0.5 , 1.0, 0.5, 1.0, 1.0, 0.5, 1.0   // BALL 4
};


//---------------------------------------------------------------------------------
void drawSpriteTex( int x, int y, int width, int height, int image ) {
//---------------------------------------------------------------------------------

	int texIndex = image * 8;      // choose which 
	
	printf("starting quad draw");
	
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	printf("in quad");
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(texCoords[texIndex],texCoords[texIndex+1]);
		
		texIndex+=2;
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(texCoords[texIndex],texCoords[texIndex+1]);
		
		texIndex+=2;
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(texCoords[texIndex],texCoords[texIndex+1]);
		
		texIndex+=2;
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(texCoords[texIndex],texCoords[texIndex+1]);
	GX_End();									// Done Drawing The Quad 

}
*/