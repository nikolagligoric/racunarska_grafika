#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Hud2D
{
    unsigned int numberTex[10]{};
    unsigned int doorOpenTex = 0;
    unsigned int doorClosedTex = 0;
    unsigned int controlTex = 0;

    unsigned int uiShader = 0;
    unsigned int routeShader = 0;
    unsigned int circleShader = 0;

    unsigned int vaoRoute = 0, vboRoute = 0;
    unsigned int vaoCircle = 0, vboCircle = 0;
    unsigned int vaoQuad = 0, vboQuad = 0;

    glm::mat4 panelWorld = glm::mat4(1.0f);
    glm::mat4 V = glm::mat4(1.0f);
    glm::mat4 P = glm::mat4(1.0f);
    bool hasPanel = false;

    void init(unsigned int uiShader_, unsigned int routeShader_, unsigned int circleShader_);
    void setTextures(const unsigned int numTex[10], unsigned int doorOpen, unsigned int doorClosed, unsigned int control);

    void setPanel(const glm::mat4& panelWorld_, const glm::mat4& V_, const glm::mat4& P_);

    void drawRouteAndStops();
    void drawDoorIcon(bool atStop);
    void drawControlIcon(bool controlInBus);
    void drawBusMarker(float x, float y);
    void drawPassengerCount(int count);
    void drawFineCount(int fines);
    void destroy();
};
