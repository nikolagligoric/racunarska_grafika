// Nikola Gligoric RA6/2022 - Autobus 3D
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "Hud2D.h"
#include "RouteData.h"
#include "BusLogic.h"

#include "shader.hpp"
#include "model.hpp"
#include "BusRender.h"

const glm::vec4 COL_WALL = glm::vec4(0.95f, 0.85f, 0.20f, 1.0f);
const glm::vec4 COL_FLOOR = glm::vec4(0.03f, 0.03f, 0.03f, 1.0f);
const glm::vec4 COL_FRAME = glm::vec4(0.95f, 0.85f, 0.20f, 1.0f);
const glm::vec4 COL_PANEL = glm::vec4(0.03f, 0.03f, 0.03f, 1.0f);
const glm::vec4 COL_DOOR = glm::vec4(0.95f, 0.85f, 0.20f, 1.0f);
const glm::vec4 COL_ROOF = glm::vec4(0.88f, 0.90f, 0.93f, 1.0f);

static bool depthEnabled = true;
static bool cullEnabled = true;

static bool prevLMB = false;
static bool prevRMB = false;
static bool prevK = false;

static unsigned int numberTex[10]{};
static unsigned int controlTex = 0;
static unsigned int doorOpenTex = 0;
static unsigned int doorClosedTex = 0;

static int gW = 800, gH = 600;

static float camYaw = -90.0f;
static float camPitch = 0.0f;
static float mouseSens = 0.08f;

static bool   firstMouse = true;
static double lastMX = 0.0, lastMY = 0.0;
static float  shakePhase = 0.0f;
static float  wheelSteer = 0.0f;

static glm::vec3 camPos = glm::vec3(0.0f, 1.10f, 0.35f);

static void cursorPosCallback(GLFWwindow*, double mx, double my)
{
    if (firstMouse) { lastMX = mx; lastMY = my; firstMouse = false; }

    double dx = mx - lastMX;
    double dy = lastMY - my;
    lastMX = mx;
    lastMY = my;

    camYaw += (float)dx * mouseSens;
    camPitch += (float)dy * mouseSens;

    camPitch = glm::clamp(camPitch, -60.0f, 60.0f);
    camYaw = glm::clamp(camYaw, -180.0f, 0.0f);
}

static glm::mat4 MakeProjection()
{
    float aspect = (gH == 0) ? 1.0f : (float)gW / (float)gH;
    return glm::perspective(glm::radians(75.0f), aspect, 0.1f, 100.0f);
}

static void framebufferSizeCallback(GLFWwindow*, int w, int h)
{
    gW = w;
    gH = h;
    glViewport(0, 0, w, h);
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_1) { depthEnabled = true;  glEnable(GL_DEPTH_TEST); }
    if (key == GLFW_KEY_2) { depthEnabled = false; glDisable(GL_DEPTH_TEST); }

    if (key == GLFW_KEY_3) { cullEnabled = true;   glEnable(GL_CULL_FACE); }
    if (key == GLFW_KEY_4) { cullEnabled = false;  glDisable(GL_CULL_FACE); }
}

static unsigned int preprocessTexture(const char* filepath)
{
    unsigned int texture = loadImageToTexture(filepath);
    if (!texture) return 0;

    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

int main()
{
    srand((unsigned)time(nullptr));
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    gW = mode->width;
    gH = mode->height;

    GLFWwindow* window = glfwCreateWindow(gW, gH, "Autobus 3D", monitor, nullptr);
    if (!window) return -2;

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) return -3;

    glViewport(0, 0, gW, gH);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int shader = createShader("basic.vert", "basic.frag");
    unsigned int uiShader = createShader("ui.vert", "ui.frag");
    unsigned int routeShader = createShader("route.vert", "route.frag");
    unsigned int circleShader = createShader("circle.vert", "circle.frag");

    Shader modelShader("model.vert", "model.frag");
    modelShader.use();
    modelShader.setInt("uDiffMap1", 0);

    glUseProgram(uiShader);
    glUniform1i(glGetUniformLocation(uiShader, "uTexture"), 0);

    unsigned int nameTex = preprocessTexture("res/ime.png");

    for (int i = 0; i < 10; i++) {
        char path[64];
        sprintf(path, "res/%d.png", i);
        numberTex[i] = preprocessTexture(path);
    }
    controlTex = preprocessTexture("res/kontrola.png");
    doorOpenTex = preprocessTexture("res/opened_door.png");
    doorClosedTex = preprocessTexture("res/closed_door.png");

    Hud2D hud;
    hud.init(uiShader, routeShader, circleShader);
    hud.setTextures(numberTex, doorOpenTex, doorClosedTex, controlTex);

    unsigned int VAOName = 0, VBOName = 0;
    {
        float nameVerts[] = {
            -0.98f, 0.98f,  0.0f, 1.0f,
            -0.98f, 0.80f,  0.0f, 0.0f,
            -0.55f, 0.80f,  1.0f, 0.0f,

            -0.98f, 0.98f,  0.0f, 1.0f,
            -0.55f, 0.80f,  1.0f, 0.0f,
            -0.55f, 0.98f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAOName);
        glGenBuffers(1, &VBOName);

        glBindVertexArray(VAOName);
        glBindBuffer(GL_ARRAY_BUFFER, VBOName);
        glBufferData(GL_ARRAY_BUFFER, sizeof(nameVerts), nameVerts, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    glUseProgram(shader);

    int loc_uM = glGetUniformLocation(shader, "uM");
    int loc_uV = glGetUniformLocation(shader, "uV");
    int loc_uP = glGetUniformLocation(shader, "uP");
    int loc_transparent = glGetUniformLocation(shader, "transparent");

    int loc_uLightPos = glGetUniformLocation(shader, "uLightPos");
    int loc_uLightColor = glGetUniformLocation(shader, "uLightColor");
    int loc_uViewPos = glGetUniformLocation(shader, "uViewPos");
    int loc_uTint = glGetUniformLocation(shader, "uTint");
    std::cout << "loc_uTint = " << loc_uTint << std::endl;

    glUseProgram(shader);
    glUniform4f(loc_uTint, 1.0f, 1.0f, 1.0f, 1.0f);

    float vertices3D[] =
    {
        -0.5f,-0.5f,-0.5f,  1,1,1,1,   0,0,-1,
         0.5f, 0.5f,-0.5f,  1,1,1,1,   0,0,-1,
         0.5f,-0.5f,-0.5f,  1,1,1,1,   0,0,-1,
        -0.5f,-0.5f,-0.5f,  1,1,1,1,   0,0,-1,
        -0.5f, 0.5f,-0.5f,  1,1,1,1,   0,0,-1,
         0.5f, 0.5f,-0.5f,  1,1,1,1,   0,0,-1,

        -0.5f,-0.5f, 0.5f,  1,1,1,1,   0,0, 1,
         0.5f,-0.5f, 0.5f,  1,1,1,1,   0,0, 1,
         0.5f, 0.5f, 0.5f,  1,1,1,1,   0,0, 1,
        -0.5f,-0.5f, 0.5f,  1,1,1,1,   0,0, 1,
         0.5f, 0.5f, 0.5f,  1,1,1,1,   0,0, 1,
        -0.5f, 0.5f, 0.5f,  1,1,1,1,   0,0, 1,

        -0.5f, 0.5f, 0.5f,  1,1,1,1,  -1,0,0,
        -0.5f, 0.5f,-0.5f,  1,1,1,1,  -1,0,0,
        -0.5f,-0.5f,-0.5f,  1,1,1,1,  -1,0,0,
        -0.5f,-0.5f,-0.5f,  1,1,1,1,  -1,0,0,
        -0.5f,-0.5f, 0.5f,  1,1,1,1,  -1,0,0,
        -0.5f, 0.5f, 0.5f,  1,1,1,1,  -1,0,0,

         0.5f, 0.5f, 0.5f,  1,1,1,1,   1,0,0,
         0.5f,-0.5f,-0.5f,  1,1,1,1,   1,0,0,
         0.5f, 0.5f,-0.5f,  1,1,1,1,   1,0,0,
         0.5f,-0.5f,-0.5f,  1,1,1,1,   1,0,0,
         0.5f, 0.5f, 0.5f,  1,1,1,1,   1,0,0,
         0.5f,-0.5f, 0.5f,  1,1,1,1,   1,0,0,

        -0.5f,-0.5f,-0.5f,  1,1,1,1,   0,-1,0,
         0.5f,-0.5f,-0.5f,  1,1,1,1,   0,-1,0,
         0.5f,-0.5f, 0.5f,  1,1,1,1,   0,-1,0,
         0.5f,-0.5f, 0.5f,  1,1,1,1,   0,-1,0,
        -0.5f,-0.5f, 0.5f,  1,1,1,1,   0,-1,0,
        -0.5f,-0.5f,-0.5f,  1,1,1,1,   0,-1,0,

        -0.5f, 0.5f,-0.5f,  1,1,1,1,   0, 1,0,
         0.5f, 0.5f, 0.5f,  1,1,1,1,   0, 1,0,
         0.5f, 0.5f,-0.5f,  1,1,1,1,   0, 1,0,
         0.5f, 0.5f, 0.5f,  1,1,1,1,   0, 1,0,
        -0.5f, 0.5f,-0.5f,  1,1,1,1,   0, 1,0,
        -0.5f, 0.5f, 0.5f,  1,1,1,1,   0, 1,0,
    };

    const unsigned int stride3D = (3 + 4 + 3) * sizeof(float);

    unsigned int VAO3D, VBO3D;
    glGenVertexArrays(1, &VAO3D);
    glGenBuffers(1, &VBO3D);

    glBindVertexArray(VAO3D);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3D), vertices3D, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride3D, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride3D, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride3D, (void*)((3 + 4) * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    RenderCtx rctx;
    rctx.shader = shader;
    rctx.loc_uM = loc_uM;
    rctx.loc_uV = loc_uV;
    rctx.loc_uP = loc_uP;
    rctx.loc_transparent = loc_transparent;
    rctx.loc_uLightPos = loc_uLightPos;
    rctx.loc_uLightColor = loc_uLightColor;
    rctx.loc_uViewPos = loc_uViewPos;
    rctx.loc_uTint = loc_uTint;
    rctx.modelShader = &modelShader;
    rctx.VAO3D = VAO3D;
    rctx.cullEnabled = cullEnabled;
    rctx.COL_WALL = COL_WALL;
    rctx.COL_FLOOR = COL_FLOOR;
    rctx.COL_FRAME = COL_FRAME;
    rctx.COL_PANEL = COL_PANEL;
    rctx.COL_DOOR = COL_DOOR;
    rctx.COL_ROOF = COL_ROOF;

    BusLogic logic;
    double lastTime = glfwGetTime();
    const double TARGET_DT = 1.0 / 75.0;

    Model steeringWheel("res/Models/Steeringwheel.glb");
    Model controlModel("res/Models/control.fbx");

    std::vector<std::string> fbxPaths = {
        "res/Models/character-a.fbx",
        "res/Models/character-b.fbx",
        "res/Models/character-c.fbx",
        "res/Models/character-d.fbx",
        "res/Models/character-e.fbx",
        "res/Models/character-f.fbx",
        "res/Models/character-g.fbx",
        "res/Models/character-h.fbx",
        "res/Models/character-i.fbx",
        "res/Models/character-k.fbx",
        "res/Models/character-l.fbx",
        "res/Models/character-m.fbx",
        "res/Models/character-n.fbx",
        "res/Models/character-o.fbx",
        "res/Models/character-p.fbx",
        "res/Models/character-q.fbx",
        "res/Models/character-r.fbx"
    };

    std::vector<Model> people;
    people.reserve(fbxPaths.size());
    for (const auto& path : fbxPaths)
        people.emplace_back(path);

    while (!glfwWindowShouldClose(window))
    {
        double frameStart = glfwGetTime();

        double now = glfwGetTime();
        double dtSim = now - lastTime;
        lastTime = now;

        logic.update(now, dtSim);
        const auto& st = logic.state();

        int ii0 = st.currentRoutePoint;
        int ii1 = (ii0 + 1) % ROUTE_POINT_COUNT;
        int ii2 = (ii1 + 1) % ROUTE_POINT_COUNT;

        glm::vec2 a(route2D[ii0 * 2 + 0], route2D[ii0 * 2 + 1]);
        glm::vec2 b(route2D[ii1 * 2 + 0], route2D[ii1 * 2 + 1]);
        glm::vec2 c(route2D[ii2 * 2 + 0], route2D[ii2 * 2 + 1]);

        glm::vec2 v1 = glm::normalize(b - a);
        glm::vec2 v2 = glm::normalize(c - b);

        float cross = v1.x * v2.y - v1.y * v2.x;
        cross = glm::clamp(cross, -1.0f, 1.0f);

        float targetSteer = 0.0f;
        if (!st.atStop) targetSteer = cross * 28.0f;
        wheelSteer = glm::mix(wheelSteer, targetSteer, (float)(dtSim * 8.0));

        float shakeY = 0.0f;
        if (!st.atStop)
        {
            shakePhase += (float)(dtSim * 10.0f);
            shakeY = 0.008f * sinf(shakePhase) + 0.004f * sinf(shakePhase * 2.3f);
        }
        glm::vec3 busOffset = glm::vec3(0.55f, shakeY, 0.0f);

        glm::mat4 P = MakeProjection();

        glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 front;
        front.x = cos(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        front.y = sin(glm::radians(camPitch));
        front.z = sin(glm::radians(camYaw)) * cos(glm::radians(camPitch));
        front = glm::normalize(front);

        glm::mat4 Vcam = glm::lookAt(camPos, camPos + front, glm::vec3(0, 1, 0));

        glm::vec3 lightPos = glm::vec3(busOffset.x + 0.0f, busOffset.y + 1.55f, busOffset.z + 0.5f);

        SceneState scene;
        scene.camPos = camPos;
        scene.Vcam = Vcam;
        scene.P = P;
        scene.busOffset = busOffset;
        scene.lightPos = lightPos;

        BusRender::DrawWorldAndBus(rctx, logic, scene);

        BusRender::DrawSteeringWheel(rctx, scene, steeringWheel, wheelSteer, 25.0f);

        BusRender::DrawActors(rctx, scene, controlModel, people,
            logic.insideActors(),
            logic.hasMovingActor(),
            logic.movingActor()
        );

        glBindVertexArray(0);

        GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);
        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glUseProgram(uiShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, nameTex);
        glBindVertexArray(VAOName);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        {
            glm::mat4 PanelWorld = glm::translate(glm::mat4(1.0f), busOffset) * scene.PanelLocal;
            hud.setPanel(PanelWorld, Vcam, P);
        }

        int i0 = st.currentRoutePoint;
        int i1 = (i0 + 1) % ROUTE_POINT_COUNT;

        float cx = route2D[i0 * 2 + 0];
        float cy = route2D[i0 * 2 + 1];
        float nx = route2D[i1 * 2 + 0];
        float ny = route2D[i1 * 2 + 1];

        float bx = cx + (nx - cx) * st.travelT;
        float by = cy + (ny - cy) * st.travelT;

        hud.drawRouteAndStops();
        hud.drawBusMarker(bx, by);
        hud.drawDoorIcon(st.atStop);
        hud.drawControlIcon(st.controlInside);
        hud.drawPassengerCount(st.passengers);
        hud.drawFineCount(st.totalFines);

        if (wasCull && cullEnabled)    glEnable(GL_CULL_FACE);
        if (wasDepth && depthEnabled)  glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();

        bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        bool k = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;

        if (lmb && !prevLMB) logic.tryPassengerEnter();
        if (rmb && !prevRMB) logic.tryPassengerExit();
        if (k && !prevK)   logic.tryControlEnter();

        prevLMB = lmb;
        prevRMB = rmb;
        prevK = k;

        double dt = glfwGetTime() - frameStart;
        double remaining = TARGET_DT - dt;
        if (remaining > 0.0)
        {
            if (remaining > 0.002)
            {
                int ms = (int)((remaining - 0.001) * 1000.0);
                if (ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            }
            while (glfwGetTime() - frameStart < TARGET_DT) {}
        }
    }

    if (nameTex) glDeleteTextures(1, &nameTex);

    for (int i = 0; i < 10; i++)
        if (numberTex[i]) glDeleteTextures(1, &numberTex[i]);
    if (controlTex)    glDeleteTextures(1, &controlTex);
    if (doorOpenTex)   glDeleteTextures(1, &doorOpenTex);
    if (doorClosedTex) glDeleteTextures(1, &doorClosedTex);

    glDeleteBuffers(1, &VBOName);
    glDeleteVertexArrays(1, &VAOName);

    glDeleteBuffers(1, &VBO3D);
    glDeleteVertexArrays(1, &VAO3D);

    glDeleteProgram(shader);

    hud.destroy();
    glDeleteProgram(uiShader);
    glDeleteProgram(routeShader);
    glDeleteProgram(circleShader);

    glfwTerminate();
    return 0;
}
