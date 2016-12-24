#ifndef SUNSET_SUNRISE_H
#define SUNSET_SUNRISE_H

#include <math.h>

#define DEG_TO_RAD(deg) (M_PI * (deg) / 180.0)
#define RAD_TO_DEG(rad) ((rad) * 180.0 / M_PI)

#define CALC_SUNRISE 1
#define CALC_SUNSET 0

int dayOfYear(int year, int month, int day);

#endif