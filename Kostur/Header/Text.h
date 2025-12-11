#pragma once
#include <map>
#include <string>
#include <GL/glew.h>

struct Character {
    unsigned int TextureID;
    int SizeX, SizeY;
    int BearingX, BearingY;
    unsigned int Advance;
};

extern std::map<char, Character> Characters;
extern unsigned int TextVAO, TextVBO;

bool InitTextRendering(const char* fontPath);
void RenderText(unsigned int shader, std::string text, float x, float y, float scale);
