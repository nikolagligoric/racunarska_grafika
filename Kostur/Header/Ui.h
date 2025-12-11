#pragma once
#include <GL/glew.h>

extern unsigned int VAOidCard;
extern unsigned int VAOnumbers;
extern unsigned int numberTextures[10];

extern float idVertices[24];

void initIDCardVAO();
void drawIDCard(unsigned int shader, unsigned int texture);

void initNumberVAO();
void drawNumber(unsigned int shader, int num, float x, float y);
