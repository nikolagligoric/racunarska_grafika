#include "Hud2D.h"
#include <cmath>
#include <vector>
#include <algorithm>

extern const int ROUTE_POINT_COUNT;
extern const float route2D[];
extern const int STOP_COUNT;
extern const int stopIndices[];

#define NUM_SLICES 32

static void buildCircle(float* outVerts)
{
    outVerts[0] = 0.0f; outVerts[1] = 0.0f;
    for (int i = 1; i < NUM_SLICES + 2; i++)
    {
        float a = (i - 1) * 2.0f * 3.14159f / (float)NUM_SLICES;
        outVerts[i * 2 + 0] = cos(a);
        outVerts[i * 2 + 1] = sin(a);
    }
}

static glm::vec2 projectToNDC(const glm::vec3& world, const glm::mat4& V, const glm::mat4& P)
{
    glm::vec4 clip = P * V * glm::vec4(world, 1.0f);
    if (fabs(clip.w) < 1e-6f) return glm::vec2(0.0f);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    return glm::vec2(ndc.x, ndc.y);
}

static glm::vec3 uvToPanelLocal(float u, float v)
{
    const float V_LIFT = 0.08f;

    u = glm::clamp(u, 0.0f, 1.0f);
    v = glm::clamp(v + V_LIFT, 0.0f, 1.0f);

    const float zFace = +0.5f;
    const float eps = 0.02f;

    float x = -0.5f + u;
    float y = -0.5f + v;

    return glm::vec3(x, y, zFace + eps);
}

static void drawTexturedQuadOnPanel(Hud2D* hud, GLuint tex, float u0, float v0, float u1, float v1)
{
    if (!hud->hasPanel || !tex) return;

    glm::vec3 lTL = uvToPanelLocal(u0, v1);
    glm::vec3 lTR = uvToPanelLocal(u1, v1);
    glm::vec3 lBR = uvToPanelLocal(u1, v0);
    glm::vec3 lBL = uvToPanelLocal(u0, v0);

    auto toWorld = [&](const glm::vec3& lp) {
        return glm::vec3(hud->panelWorld * glm::vec4(lp, 1.0f));
        };

    glm::vec2 pTL = projectToNDC(toWorld(lTL), hud->V, hud->P);
    glm::vec2 pTR = projectToNDC(toWorld(lTR), hud->V, hud->P);
    glm::vec2 pBR = projectToNDC(toWorld(lBR), hud->V, hud->P);
    glm::vec2 pBL = projectToNDC(toWorld(lBL), hud->V, hud->P);

    float verts[] = {
        pTL.x, pTL.y, 0,1,
        pBL.x, pBL.y, 0,0,
        pBR.x, pBR.y, 1,0,

        pTL.x, pTL.y, 0,1,
        pBR.x, pBR.y, 1,0,
        pTR.x, pTR.y, 1,1
    };

    glUseProgram(hud->uiShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(hud->vaoQuad);
    glBindBuffer(GL_ARRAY_BUFFER, hud->vboQuad);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Hud2D::init(unsigned int uiShader_, unsigned int routeShader_, unsigned int circleShader_)
{
    uiShader = uiShader_;
    routeShader = routeShader_;
    circleShader = circleShader_;

    glGenVertexArrays(1, &vaoRoute);
    glGenBuffers(1, &vboRoute);
    glBindVertexArray(vaoRoute);
    glBindBuffer(GL_ARRAY_BUFFER, vboRoute);
    glBufferData(GL_ARRAY_BUFFER, ROUTE_POINT_COUNT * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    float circleVerts[(NUM_SLICES + 2) * 2];
    buildCircle(circleVerts);

    glGenVertexArrays(1, &vaoCircle);
    glGenBuffers(1, &vboCircle);
    glBindVertexArray(vaoCircle);
    glBindBuffer(GL_ARRAY_BUFFER, vboCircle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVerts), circleVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &vaoQuad);
    glGenBuffers(1, &vboQuad);
    glBindVertexArray(vaoQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glUseProgram(uiShader);
    glUniform1i(glGetUniformLocation(uiShader, "uTexture"), 0);
}

void Hud2D::setTextures(const unsigned int numTex[10], unsigned int doorOpen, unsigned int doorClosed, unsigned int control)
{
    for (int i = 0; i < 10; i++) numberTex[i] = numTex[i];
    doorOpenTex = doorOpen;
    doorClosedTex = doorClosed;
    controlTex = control;
}

void Hud2D::setPanel(const glm::mat4& panelWorld_, const glm::mat4& V_, const glm::mat4& P_)
{
    panelWorld = panelWorld_;
    V = V_;
    P = P_;
    hasPanel = true;
}

void Hud2D::drawRouteAndStops()
{
    if (!hasPanel) return;

    const float uMin = 0.42f, uMax = 0.97f;
    const float vMin = 0.32f, vMax = 0.93f;

    std::vector<float> mapped(ROUTE_POINT_COUNT * 2);

    for (int i = 0; i < ROUTE_POINT_COUNT; i++)
    {
        float xN = route2D[i * 2 + 0];
        float yN = route2D[i * 2 + 1];

        float uNorm = (xN + 1.0f) * 0.5f;
        float vNorm = (yN + 1.0f) * 0.5f;

        float u = uMin + (uMax - uMin) * uNorm;
        float v = vMin + (vMax - vMin) * vNorm;

        glm::vec3 lp = uvToPanelLocal(u, v);
        glm::vec3 wp = glm::vec3(panelWorld * glm::vec4(lp, 1.0f));
        glm::vec2 ndc = projectToNDC(wp, V, P);

        mapped[i * 2 + 0] = ndc.x;
        mapped[i * 2 + 1] = ndc.y;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboRoute);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(mapped.size() * sizeof(float)), mapped.data());

    glUseProgram(routeShader);
    glBindVertexArray(vaoRoute);
    glLineWidth(4.0f);
    glDrawArrays(GL_LINE_LOOP, 0, ROUTE_POINT_COUNT);
    glBindVertexArray(0);

    for (int i = 0; i < STOP_COUNT; i++)
    {
        int ridx = stopIndices[i];

        float xN = route2D[ridx * 2 + 0];
        float yN = route2D[ridx * 2 + 1];

        float uNorm = (xN + 1.0f) * 0.5f;
        float vNorm = (yN + 1.0f) * 0.5f;

        float u = uMin + (uMax - uMin) * uNorm;
        float v = vMin + (vMax - vMin) * vNorm;

        glm::vec3 lp = uvToPanelLocal(u, v);
        glm::vec3 wp = glm::vec3(panelWorld * glm::vec4(lp, 1.0f));
        glm::vec2 ndc = projectToNDC(wp, V, P);

        glUseProgram(circleShader);
        glUniform2f(glGetUniformLocation(circleShader, "uPos"), ndc.x, ndc.y);
        glUniform1f(glGetUniformLocation(circleShader, "uScale"), 0.018f);
        glUniform3f(glGetUniformLocation(circleShader, "uColor"), 1.0f, 0.0f, 0.0f);
        glBindVertexArray(vaoCircle);
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_SLICES + 2);
        glBindVertexArray(0);

        int num = i % 10;
        if (!numberTex[num]) continue;

        float du = 0.020f, dv = 0.020f;
        drawTexturedQuadOnPanel(this, numberTex[num], u - du, v - dv, u + du, v + dv);
    }
}


void Hud2D::drawDoorIcon(bool atStop)
{
    unsigned int tex = atStop ? doorOpenTex : doorClosedTex;
    if (!tex) return;

    float u0 = 0.86f, v0 = 0.15f;
    float u1 = u0 + 0.12f;
    float v1 = v0 + 0.26f;

    drawTexturedQuadOnPanel(this, tex, u0, v0, u1, v1);
}


void Hud2D::drawControlIcon(bool controlInBus)
{
    if (!controlInBus || !controlTex) return;

    float u0 = 0.60f;
    float v0 = 0.15f;
    float u1 = u0 + 0.11f;
    float v1 = v0 + 0.26f;

    drawTexturedQuadOnPanel(this, controlTex, u0, v0, u1, v1);
}

void Hud2D::drawBusMarker(float x, float y)
{
    if (!hasPanel) return;

    const float uMin = 0.42f, uMax = 0.97f;
    const float vMin = 0.32f, vMax = 0.93f;

    float uNorm = (x + 1.0f) * 0.5f;
    float vNorm = (y + 1.0f) * 0.5f;

    float u = uMin + (uMax - uMin) * uNorm;
    float v = vMin + (vMax - vMin) * vNorm;

    glm::vec3 lp = uvToPanelLocal(u, v);
    glm::vec3 wp = glm::vec3(panelWorld * glm::vec4(lp, 1.0f));
    glm::vec2 ndc = projectToNDC(wp, V, P);

    glUseProgram(circleShader);
    glUniform2f(glGetUniformLocation(circleShader, "uPos"), ndc.x, ndc.y);
    glUniform1f(glGetUniformLocation(circleShader, "uScale"), 0.014f);
    glUniform3f(glGetUniformLocation(circleShader, "uColor"), 0.2f, 0.8f, 1.0f);

    glBindVertexArray(vaoCircle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_SLICES + 2);
    glBindVertexArray(0);
}



void Hud2D::drawPassengerCount(int count)
{
    if (count < 0) count = 0;
    if (count > 99) count = 99;

    int tens = (count / 10) % 10;
    int ones = count % 10;

    if (!numberTex[tens] || !numberTex[ones]) return;

    float u0 = 0.48f;
    float v0 = 0.15f;

    float du = 0.06f;
    float dv = 0.12f;
    float gap = du * 0.15f;

    drawTexturedQuadOnPanel(this, numberTex[tens], u0, v0, u0 + du, v0 + dv);
    drawTexturedQuadOnPanel(this, numberTex[ones], u0 + du + gap, v0, u0 + 2.0f * du + gap, v0 + dv);
}

void Hud2D::drawFineCount(int fines)
{
    if (fines < 0) fines = 0;
    if (fines > 99) fines = 99;

    int tens = (fines / 10) % 10;
    int ones = fines % 10;

    if (!numberTex[tens] || !numberTex[ones]) return;

    float u0 = 0.73f;
    float v0 = 0.15f;

    float du = 0.06f;
    float dv = 0.12f;
    float gap = du * 0.15f;

    drawTexturedQuadOnPanel(this, numberTex[tens], u0, v0, u0 + du, v0 + dv);
    drawTexturedQuadOnPanel(this, numberTex[ones], u0 + du + gap, v0, u0 + 2.0f * du + gap, v0 + dv);
}



void Hud2D::destroy()
{
    if (vboRoute) glDeleteBuffers(1, &vboRoute);
    if (vaoRoute) glDeleteVertexArrays(1, &vaoRoute);

    if (vboCircle) glDeleteBuffers(1, &vboCircle);
    if (vaoCircle) glDeleteVertexArrays(1, &vaoCircle);

    if (vboQuad) glDeleteBuffers(1, &vboQuad);
    if (vaoQuad) glDeleteVertexArrays(1, &vaoQuad);

    vboRoute = vaoRoute = 0;
    vboCircle = vaoCircle = 0;
    vboQuad = vaoQuad = 0;

    hasPanel = false;
}
