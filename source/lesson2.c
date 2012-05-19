/*---------------------------------------------------------------------------------

	nehe lesson 2 port to GX by WinterMute

---------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <../include/functions.h>
 
#define DEFAULT_FIFO_SIZE	(256*1024)



static void *frameBuffer[2] = { NULL, NULL};
GXRModeObj *rmode;

u32	fb = 0; 	// initial framebuffer index
GXColor background;





void initialize() {
	f32 yscale;

	u32 xfbHeight;

	Mtx view;
	Mtx44 perspective;

	
	background = (GXColor){0, 0xff, 0, 0xff};


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



//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
//---------------------------------------------------------------------------------
	initialize();
	
	// Initialise the console, required for printf
    
	
	while(1) {
		console_init(frameBuffer[fb],20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
		WPAD_ScanPads();
		int buttons = WPAD_ButtonsDown(0);

		if (buttons) {
			if (buttons & WPAD_BUTTON_HOME) exit(0);
			
			changeColorBasedOnButtons(buttons);
			
			
			// OMG I JUST REALIZED I SHOULD MAKE IT SO MOVING THE JOYSTICK PANS THE COLORS!!!!!
			// THAT WOULD MAKE Q SOOOO HAPPY!!!!!
			
			//char buf[5];
			//sprintf(buf, "%d", buttons);
			//printf(buf);
			
			printf("%s\n", byte_to_binary(buttons));

			
			
			
			
		}
		
		
		
		fb ^= 1;		// flip framebuffer
		//GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		//GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer[fb],GX_TRUE);

		VIDEO_SetNextFramebuffer(frameBuffer[fb]);
 
		//VIDEO_Flush();
		
		VIDEO_WaitVSync();


	}
	return 0;
}

