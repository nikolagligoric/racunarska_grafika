#include "../Header/Bus.h"
#include "../Header/UI.h"

unsigned int VAObus;
unsigned int VBOBus;
unsigned int VAOdoors;
unsigned int VBOdoors;
unsigned int busTexture;
unsigned int doorClosedTexture;
unsigned int doorOpenTexture;

void initBusVAO()
{
    glGenVertexArrays(1, &VAObus);
    glGenBuffers(1, &VBOBus);

    glBindVertexArray(VAObus);
    glBindBuffer(GL_ARRAY_BUFFER, VBOBus);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void initDoorsVAO()
{
    glGenVertexArrays(1, &VAOdoors);
    glGenBuffers(1, &VBOdoors);

    glBindVertexArray(VAOdoors);
    glBindBuffer(GL_ARRAY_BUFFER, VBOdoors);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawBus(unsigned int shader, float sx, float sy)
{
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, busTexture);

    float w = 0.08f;
    float h = 0.10f;

    float verts[] = {
        sx - w, sy + h, 0,1,
        sx - w, sy - h, 0,0,
        sx + w, sy - h, 1,0,
        sx - w, sy + h, 0,1,
        sx + w, sy - h, 1,0,
        sx + w, sy + h, 1,1
    };

    glBindVertexArray(VAObus);
    glBindBuffer(GL_ARRAY_BUFFER, VBOBus);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawDoors(unsigned int shader, unsigned int tex)
{
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    float x = 0.85f;
    float y = -0.95f;
    float w = 0.12f;
    float h = 0.18f;

    float verts[] = {
        x,     y + h, 0.0f, 1.0f,
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,

        x,     y + h, 0.0f, 1.0f,
        x + w, y,     1.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f
    };

    glBindVertexArray(VAOdoors);
    glBindBuffer(GL_ARRAY_BUFFER, VBOdoors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawControl(unsigned int shader, unsigned int tex)
{
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    float w = 0.10f;
    float h = 0.18f;

    float x = -0.95f;
    float y = -0.95f;

    float verts[] = {
        x,     y + h, 0.0f, 1.0f,
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,

        x,     y + h, 0.0f, 1.0f,
        x + w, y,     1.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f
    };

    glBindVertexArray(VAOdoors);
    glBindBuffer(GL_ARRAY_BUFFER, VBOdoors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}



