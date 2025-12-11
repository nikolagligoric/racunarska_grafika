#pragma once
#include <GL/glew.h>

extern float routeVertices[24];
extern int stopIndices[10];
extern const int STOP_COUNT;

extern unsigned int VAOroute;
extern unsigned int VAOcircle;

void initRouteVAO();
void initCircleVAO();

void drawRoute(unsigned int shader);
void drawCircle(unsigned int shader, float sx, float sy, float radius);
