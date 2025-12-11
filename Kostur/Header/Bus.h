#pragma once
#include <GL/glew.h>

extern unsigned int busTexture;
extern unsigned int doorClosedTexture;
extern unsigned int doorOpenTexture;
extern unsigned int VAOdoors;
extern unsigned int VBOdoors;

void initBusVAO();
void initDoorsVAO();
void drawBus(unsigned int shader, float x, float y);
void drawDoors(unsigned int shader, unsigned int tex);
void drawControl(unsigned int shader, unsigned int tex);


