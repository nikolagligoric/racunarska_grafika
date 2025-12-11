#include "../Header/Route.h"
#include <cmath>

#define NUM_SLICES 32

float routeVertices[24] = {
    -0.6f, -0.7f,
     0.3f, -0.5f,
     0.4f, -0.2f,
     0.4f,  0.0f,
     0.6f,  0.1f,
     0.6f,  0.4f,
     0.4f,  0.4f,
     0.4f,  0.6f,
    -0.1f,  0.7f,
    -0.3f,  0.6f,
    -0.6f,  0.6f,
    -0.5f,  0.0f,
};

int stopIndices[10] = { 0,1,2,4,5,7,8,9,10,11 };
const int STOP_COUNT = 10;

unsigned int VAOroute, VBOroute;

unsigned int VAOcircle, VBOcircle;

void initRouteVAO()
{
    glGenVertexArrays(1, &VAOroute);
    glGenBuffers(1, &VBOroute);

    glBindVertexArray(VAOroute);
    glBindBuffer(GL_ARRAY_BUFFER, VBOroute);
    glBufferData(GL_ARRAY_BUFFER, sizeof(routeVertices), routeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawRoute(unsigned int shader)
{
    glUseProgram(shader);
    glBindVertexArray(VAOroute);
    glLineWidth(5.0f);
    glDrawArrays(GL_LINE_LOOP, 0, 12);
}

void initCircleVAO()
{
    float circleVertices[(NUM_SLICES + 2) * 2];

    circleVertices[0] = 0.0f;
    circleVertices[1] = 0.0f;

    for (int i = 1; i < NUM_SLICES + 2; i++)
    {
        float angle = (i - 1) * 2.0f * 3.14159f / NUM_SLICES;
        circleVertices[i * 2] = cos(angle);
        circleVertices[i * 2 + 1] = sin(angle);
    }

    glGenVertexArrays(1, &VAOcircle);
    glGenBuffers(1, &VBOcircle);

    glBindVertexArray(VAOcircle);
    glBindBuffer(GL_ARRAY_BUFFER, VBOcircle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void drawCircle(unsigned int shader, float sx, float sy, float radius)
{
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader, "uPos"), sx, sy);
    glUniform1f(glGetUniformLocation(shader, "uScale"), radius);
    glUniform3f(glGetUniformLocation(shader, "uColor"), 1.0f, 0.0f, 0.0f);

    glBindVertexArray(VAOcircle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_SLICES + 2);
}