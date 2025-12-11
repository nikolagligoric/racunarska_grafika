#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "../Header/Util.h"
#include "../Header/UI.h"
#include "../Header/Route.h"
#include "../Header/Bus.h"
#include "../Header/Text.h"

int passengers = 0;
int screenWidth;
int screenHeight;
double lastFrameTime = 0.0;
bool atStop = true;
double stopStartTime = 0.0;
int currentRoutePoint = 0;
float travelT = 0.0f;
unsigned int idCardTexture;
bool controlInBus = false;
int totalFines = 0;
unsigned int controlTexture;

void preprocessTexture(unsigned& texture, const char* filepath)
{
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    screenWidth = mode->width;
    screenHeight = mode->height;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Autobus", monitor, NULL);
    if (!window) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    GLFWcursor* cursor = loadImageToCursor("Resources/kursor.png");
    if (cursor)
    {
        glfwSetCursor(window, cursor);
    }
    else
    {
        std::cout << "Greska: kursor nije ucitan!" << std::endl;
    }

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    preprocessTexture(idCardTexture, "Resources/ime.png");
    char filename[64];
    for (int i = 0; i < 10; i++) {
        sprintf_s(filename, "Resources/%d.png", i);
        preprocessTexture(numberTextures[i], filename);
    }
    preprocessTexture(busTexture, "Resources/bus.png");
    preprocessTexture(doorClosedTexture, "Resources/closed_door.png");
    preprocessTexture(doorOpenTexture, "Resources/opened_door.png");
    preprocessTexture(controlTexture, "Resources/kontrola.png");

    unsigned int uiShader = createShader("ui.vert", "ui.frag");
    glUniform1i(glGetUniformLocation(uiShader, "uTexture"), 0);

    unsigned int routeShader = createShader("route.vert", "route.frag");
    unsigned int circleShader = createShader("circle.vert", "circle.frag");
    InitTextRendering("Resources/arial.ttf");
    unsigned int textShader = createShader("text.vert", "text.frag");
    glUseProgram(textShader);
    glUniform1i(glGetUniformLocation(textShader, "text"), 0);

    unsigned int VAOid, VAOroute, VAOcircle;
    initIDCardVAO();
    initRouteVAO();
    initNumberVAO();
    initBusVAO();
    initDoorsVAO();
    initCircleVAO();

    lastFrameTime = glfwGetTime();

    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (atStop)
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                if (passengers < 50)
                    passengers++;
                while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                    glfwPollEvents();
            }

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            {
                if (passengers > 0)
                    passengers--;
                while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                    glfwPollEvents();
            }
            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            {
                if (!controlInBus && passengers < 50)
                {
                    controlInBus = true;
                    passengers++;
                    std::cout << "Kontrola usla" << std::endl;

                    while (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                        glfwPollEvents();
                }
            }

        }



        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        drawIDCard(uiShader, idCardTexture);
        drawRoute(routeShader);

        for (int i = 0; i < STOP_COUNT; i++)
        {
            float sx = routeVertices[stopIndices[i] * 2];
            float sy = routeVertices[stopIndices[i] * 2 + 1];

            drawCircle(circleShader, sx, sy, 0.04f);
            drawNumber(uiShader, i, sx, sy);
        }

        // BUS MOVEMENT LOGIC (unchanged)
        int nextRoutePoint = (currentRoutePoint + 1) % 12;
        float cx = routeVertices[currentRoutePoint * 2];
        float cy = routeVertices[currentRoutePoint * 2 + 1];
        float nx = routeVertices[nextRoutePoint * 2];
        float ny = routeVertices[nextRoutePoint * 2 + 1];

        if (atStop)
        {
            if (currentTime - stopStartTime >= 10.0)
            {
                atStop = false;
                travelT = 0.0f;
            }
        }
        else
        {
            float speed = 0.25f;
            travelT += speed * (float)deltaTime;

            if (travelT >= 1.0f)
            {
                travelT = 0.0f;
                currentRoutePoint = nextRoutePoint;

                for (int i = 0; i < STOP_COUNT; i++)
                    if (stopIndices[i] == currentRoutePoint)
                    {
                        atStop = true;
                        stopStartTime = currentTime;

                        if (controlInBus)
                        {
                            if (passengers > 1)
                            {
                                int maxFines = passengers - 1;
                                int numFines = rand() % maxFines;
                                totalFines += numFines;
                            }

                            passengers--; // kontrola izlazi
                            controlInBus = false;
                            std::cout << "Kontrola izasla. Kazne: " << totalFines << std::endl;
                        }

                        break;
                    }

            }
        }

        float bx = cx + (nx - cx) * travelT;
        float by = cy + (ny - cy) * travelT;

        drawBus(uiShader, bx, by);
        unsigned int doorTex = atStop ? doorOpenTexture : doorClosedTexture;
        drawDoors(uiShader, doorTex);
        RenderText(textShader, "Putnici: " + std::to_string(passengers),
            -0.95f, 0.92f, 0.001f);

        if (controlInBus)
        {
            drawControl(uiShader, controlTexture);
        }

        RenderText(textShader,
            "Kazne: " + std::to_string(totalFines),
            -0.95f, 0.85f, 0.001f);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(uiShader);
    glDeleteProgram(routeShader);
    glfwTerminate();
    return 0;
}
