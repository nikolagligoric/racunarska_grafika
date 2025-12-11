#include "../Header/UI.h"
#include <cstring>


unsigned int VAOidCard, VBOidCard;
unsigned int VAOnumbers, VBOnumbers;
unsigned int numberTextures[10];

float idVertices[24];

void initIDCardVAO()
{
    float w = 0.32f, h = 0.15f;
    float x = 1.0f - w, y = 1.0f - h;

    float temp[] = {
       x,1.0f, 0.0f,1.0f,
       x,y,    0.0f,0.0f,
       1.0f,y, 1.0f,0.0f,

       x,1.0f, 0.0f,1.0f,
       1.0f,y, 1.0f,0.0f,
       1.0f,1.0f,1.0f,1.0f
    };
    memcpy(idVertices, temp, sizeof(temp));

    glGenVertexArrays(1, &VAOidCard);
    glGenBuffers(1, &VBOidCard);

    glBindVertexArray(VAOidCard);
    glBindBuffer(GL_ARRAY_BUFFER, VBOidCard);
    glBufferData(GL_ARRAY_BUFFER, sizeof(idVertices), idVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void initNumberVAO()
{
    glGenVertexArrays(1, &VAOnumbers);
    glGenBuffers(1, &VBOnumbers);

    glBindVertexArray(VAOnumbers);
    glBindBuffer(GL_ARRAY_BUFFER, VBOnumbers);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawIDCard(unsigned int shader, unsigned int texture)
{
    glUseProgram(shader);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAOidCard);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawNumber(unsigned int shader, int num, float sx, float sy)
{
    glUseProgram(shader);
    glBindTexture(GL_TEXTURE_2D, numberTextures[num]);

    float size = 0.03f;
    float verts[] = {
        sx - size, sy + size, 0,1,
        sx - size, sy - size, 0,0,
        sx + size, sy - size, 1,0,
        sx - size, sy + size, 0,1,
        sx + size, sy - size, 1,0,
        sx + size, sy + size, 1,1
    };

    glBindVertexArray(VAOnumbers);
    glBindBuffer(GL_ARRAY_BUFFER, VBOnumbers);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}
