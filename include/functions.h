#ifndef functions_H
#define functions_H



void byte_to_binary(char buf[], int x);

int if_negative_be_zero(int x);
int if_positive_be_zero(int x);

double convertJoyToDegrees(int joy_x, int joy_y);
GXColor setBackgroundBasedOnDegrees(GXColor background, double degrees);

bool deadZoneClearance(int joy_x, int joy_y, int old_x, int old_y);

GXColor darkenBackgroundBasedOnDistance(GXColor background, joystick_t joy_data);


bool inDeadzone(joystick_t joy_data);
bool joyMovementNegligable(int joy_x, int joy_y, int old_x, int old_y, int tolerance);
int countUpToSixty(int x);

//void drawSpriteTex( int x, int y, int width, int height, int image );
double getDistanceOfJoystickFromOrigin(int joy_x, int joy_y);
double getDistanceOfJoystickFromCenter(joystick_t joy_data);

void printNameOfColor(double degrees);
#endif