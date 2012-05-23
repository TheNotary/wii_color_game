#ifndef functions_H
#define functions_H



void byte_to_binary(char buf[], int x);

int if_negative_be_zero(int x);
int if_positive_be_zero(int x);

double convertJoyToDegrees(int joy_x, int joy_y);
GXColor setBackgroundBasedOnDegrees(GXColor background, double degrees);

bool deadZoneClearance(int joy_x, int joy_y, int old_x, int old_y);

GXColor darkenBackgroundBasedOnDistance(GXColor background, int joy_x, int joy_y);
#endif