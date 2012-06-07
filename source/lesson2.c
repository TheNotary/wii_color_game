/*---------------------------------------------------------------------------------

	wii Color Game
	    (started by sktlz_plus)

	The wii color game is an interactive learning game intended for the 1-3 year old 
	video gamer.  It's based on color memorization and verbal communication skill building.  
	The game must be played under supervision.  The baby handler (from here on refered 
	to as handler) is presented with the challege of holding the baby and coaxing the 
	baby to hold the nunchuck.  
	
	[[Current play modes]]
	
	::Learning The Joystick
	Idealy, the handler will be able to get the baby 
	interested in what the joystick does (it changes colors on the screen).  It is 
	important that the handler feign terrific excitement about what happens to the
	screen as the baby touches the joystick (the screen color will change).  Pointing
	at the joystick and moving the babies thumb on the joystick my help keep the baby
	interested.  However, babies are to young to be 'goal oriented' and so the handler
	should not be too overly concerned with how much interest the baby takes in the 
	joystick.  
	
	::Color Shouting
	Another method of play is to simply have the handler change the color of the screen
	and shout out the color being displayed in excitement.  Ideally the baby will pick
	up on the habit and will start shouting colors too!  When the baby says the wrong 
	color, VERY soft feedback can be applied such as, "Sort of!  It's... maybe it's BLUE!"
	
	::Color Trivia
	Simply asking "what color is that?" while pointing the joystick in one of the 8 color zones.  
	
	::Finding Colors
	For older babies, possibly 3 year olds, it may be possible to ask the infant to use
	the joystick to put a color on the screen.  
	
	
	
	TODO:
	-Include a mini-map of the color wheel
	-Make a start screen
	-Impliment a menu
	-When idle, a loveable colored yoshi should apear to reclaim the babies attention
	
	-Add a number feature based on the color game (number 1-3 fit nicely but must master the minimap part)
	
	-Add a play mode where the computer asks you to seek a color
	
	-Add a mode where the color wheel is scrambled!
	
	-Add a mode where the game asks "What color is the [object]" and you move the joystick to the color
	
---------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <time.h>
#include <jpeg/jpgogc.h>
#include <wiiuse/wpad.h>
#include <../include/functions.h>
 
#define DEFAULT_FIFO_SIZE	(256*1024)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif




static void *frameBuffer[2] = { NULL, NULL};
GXRModeObj *rmode;

u32	fb = 0; 	// initial framebuffer index
GXColor background;

int joy_x;
int joy_y;

int old_x;  // for deadzone detection
int old_y;  

int my_timer = 0;
int frame_counter = 0;
bool areFlickering = false;


int randTimeTillFlicker = 10;

int gameMode = 0;  // 0 is menu...  change this to enum when I have a second mode...

double oldDegrees = -1;

Mtx view;
Mtx44 perspective;
Mtx model, modelview;


void initialize() {
	f32 yscale;
	u32 xfbHeight;
	
	background = (GXColor){0, 0x0, 0, 0xff};

	// init the vi.
	VIDEO_Init();
	WPAD_Init();
 
	rmode = VIDEO_GetPreferredMode(NULL);
	
	// allocate 2 framebuffers for double buffering
	frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(frameBuffer[fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	// setup the fifo and then init the flipper
	void *gp_fifo = NULL;
	gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gp_fifo,0,DEFAULT_FIFO_SIZE);
 
	GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);
 
	// clears the bg to color and clears the z buffer
	GX_SetCopyClear(background, 0x00ffffff);
 
	// other gx setup
	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));
 
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer[fb],GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
 

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
 
	// setup the vertex attribute table
	// describes the data
	// args: vat location 0-7, type of data, data format, size, scale
	// so for ex. in the first call we are sending position data with
	// 3 values X,Y,Z of size F32. scale sets the number of fractional
	// bits for non float data.
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
 
	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	// setup our camera at the origin
	// looking down the -z axis with y up
	guVector cam = {0.0F, 0.0F, 0.0F},
			up = {0.0F, 1.0F, 0.0F},
			look = {0.0F, 0.0F, -1.0F};

	guLookAt(view, &cam, &up, &look);
 

	// setup our projection matrix
	// this creates a perspective matrix with a view angle of 90,
	// and aspect ratio based on the display resolution
    f32 w = rmode->viWidth;
    f32 h = rmode->viHeight;
	guPerspective(perspective, 45, (f32)w/h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);
	
	srand(time(NULL));  // seed random number system based on sys time...
}


// Pass in the buttons pressed, and this function will take care of the logic
void changeColorBasedOnButtons(int buttons){
	if (buttons & WPAD_BUTTON_A) {
		background = (GXColor){0, 0, 0xff, 0xff};   // make the screen blue
	}
	if (buttons & WPAD_BUTTON_B) {
		background = (GXColor){0, 0xff, 0x00, 0xff};   // make the screen green
	}
	if (buttons & WPAD_BUTTON_1) {
		background = (GXColor){0xff, 0x00, 0x00, 0xff};   // make the screen red
	}
	if (buttons & WPAD_BUTTON_RIGHT) {
		background = (GXColor){0xff, 0xff, 0x00, 0xff};   // make the screen Yellow
	}
	if (buttons & WPAD_BUTTON_UP) {
		background = (GXColor){0xff, 0xa5, 0x00, 0xff};   // make the screen ORANGE
	}
	if (buttons & WPAD_BUTTON_LEFT) {
		background = (GXColor){0xff, 0xa5, 0x00, 0xff};   // make the screen ORANGE
	}
	if (buttons & WPAD_BUTTON_DOWN) {
		background = (GXColor){0xff, 0xa5, 0x00, 0xff};   // make the screen ORANGE
	}
	GX_SetCopyClear(background, 0x00ffffff);
}


//flickerTheScreenToGetChildsAttentionAgain();
void flickerRoutine(){
	if (frame_counter == 0){// every second... count up
		my_timer++;
		
		if (my_timer > randTimeTillFlicker){
			
			
			//printf("FLICKER");
			areFlickering = true;
			background = (GXColor){0x255 & rand(), 0x255 & rand(), 0x255 & rand(), 0xff};
			GX_SetCopyClear(background, 0x00ffffff);
			
			my_timer = 0;
			// reset random time till flickering when idle...
			double randNum = (double)rand()/(double)RAND_MAX;
			randTimeTillFlicker = (randNum * 10) + 2;
		}
		
		if (areFlickering){
			// 50% chance to return to non-flicker status... alows for variable length flickering
			if ((rand() & 1) == 1){
				// end the flickering event
				background = (GXColor){0,0,0, 0xff};
				GX_SetCopyClear(background, 0x00ffffff);
				areFlickering = false;
			}
		}
		
	}
}

void changeColorBasedOnJoystick(){
	struct expansion_t data;
	WPAD_Expansion(WPAD_CHAN_0, &data); // Get expansion info from the first wiimote
	
	joystick_t joy_data = data.nunchuk.js;
	
	
	joy_x = data.nunchuk.js.pos.x - data.nunchuk.js.center.x;
	joy_y = data.nunchuk.js.pos.y - data.nunchuk.js.center.y;
	
	
	//printf("minimum X, Y:  %d, %d   C:  %d", data.nunchuk.js.min.x, data.nunchuk.js.min.y, data.nunchuk.js.center.x);
	//printf("Center X, Y:  %d, %d  DIST: %d", data.nunchuk.js.center.x, data.nunchuk.js.center.y, (int)getDistanceOfJoystickFromCenter(joy_data));
	//int count = 60;
	//while (count--) VIDEO_WaitVSync();
	
	int tolerance = 10;
	if (joyMovementNegligable(joy_x, joy_y, old_x, old_y, tolerance) && inDeadzone(joy_data)){ // if we're in the deadzone, ignore movement up to 10...
		
		flickerRoutine();
		
		return;
	}
	old_x = joy_x;
	old_y = joy_y;
	
	
	if (inDeadzone(joy_data)){
		background = (GXColor){0,0,0,0xff};
		GX_SetCopyClear(background, 0x00ffffff);
		return;
	}
	
	
	//printf("Maxes:  %d, %d  ", joy_data.max.x, joy_data.max.y);
	//int count = 60;
	//while(count--) VIDEO_WaitVSync();
	
	double degrees;
	degrees = convertJoyToDegrees(joy_x, joy_y);
	
	if(oldDegrees - degrees > 40){            // about this code:  Sometimes, at the begining of the game, the screen will flash to red for 1/60hz... this is annoying.  It's always the same red too I think.  I found out it was the actual joystick getting picked up as being down there... To resolve the problem, the player can swivel the joystick 360 degrees and the flickering usually stops
		printf("SPIKE DETECTED %f ", degrees); // I'm not 100% sure what the nature of this bug is, but perhaps it's registering (0,0) as it's values...  but it can't, it's minimums are well above that... 
	}                                         // I think just having this code here is lucky though... cause I don't get flickering anymore...
	
	if (degrees != -1){
		GXColor *careful_bg1;
		careful_bg1 = malloc(30 * sizeof(GXColor));
		
		
		
		*careful_bg1 = setBackgroundBasedOnDegrees(background, degrees);
		*careful_bg1 = darkenBackgroundBasedOnDistance(*careful_bg1, joy_data);
		background = *careful_bg1;
		GX_SetCopyClear(background, 0x00ffffff);
		
		free(careful_bg1);
		oldDegrees = degrees;
		
		printNameOfColor(degrees);
	}
}



void showTheTitleScreen(){
	
	
}

void prepairForSeriousDrawing(){
	// do this before drawing
	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);

	guMtxIdentity(model);
	guMtxTransApply(model, model, -1.5f,0.0f,-6.0f);
	guMtxConcat(view,model,modelview);
	// load the modelview matrix into matrix memory
	GX_LoadPosMtxImm(modelview, GX_PNMTX0);
}

void drawAGoofyWhiteTriangle(){
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
		GX_Position3f32( 0.0f, 1.0f, 0.0f);		// Top
		GX_Position3f32(-1.0f,-1.0f, 0.0f);	// Bottom Left
		GX_Position3f32( 1.0f,-1.0f, 0.0f);	// Bottom Right
	GX_End();
}

// these goofy white rectangles can have textures printed on them...
// that's how I'm going to handle the title screen
void drawAGoofyWhiteRectangle(){
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32( -1.0f, 0.0f, 0.0f);		// Top Left
		GX_Position3f32( 1.0f, 0.0f, 0.0f);		// Top Right
		GX_Position3f32( 1.0f,-1.0f, 0.0f);	// Bottom Right
		GX_Position3f32(-1.0f,-1.0f, 0.0f);	// Bottom Left
	GX_End();
}






 //---------------------------------------------------------------------------------
int main( int argc, char **argv )
{//---------------------------------------------------------------------------------
	initialize();
	
	// Initialise the console, required for printf
    console_init(frameBuffer[fb],20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	while(1) {
		frame_counter = countUpToSixty(frame_counter);
		WPAD_ScanPads();
		
		int buttons = WPAD_ButtonsDown(0);

		if (buttons) {
			if (buttons & WPAD_BUTTON_HOME) exit(0);
			//changeColorBasedOnButtons(buttons);
		}
		
		
		
		prepairForSeriousDrawing();
		
		//drawAGoofyWhiteTriangle();
		
		//drawAGoofyWhiteRectangle();
		
		
		// do this stuff after drawing
		GX_DrawDone();
		
		
		
		
		switch(gameMode){
			case 0:
				// show the title screen menu
				showTheTitleScreen();
				gameMode = 1;
				break;
			case 1:
				// play that basic color game... free play mode
				changeColorBasedOnJoystick();
				break;
			case 2:
				// play the "GO TO THE COLOR SPOKEN" game
				break;
			case 3:
				// Play the number game
				break;
			default:
				printf("unknown game mode!!!");
				break;
		}
		
		fb ^= 1;		// flip framebuffer
		
		
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer[fb],GX_TRUE);

		VIDEO_SetNextFramebuffer(frameBuffer[fb]);

		VIDEO_Flush();
		
		VIDEO_WaitVSync();
	}
	return 0;
}

