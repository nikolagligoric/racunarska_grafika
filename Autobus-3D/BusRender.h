#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <deque>
#include "shader.hpp"
#include "model.hpp"
#include "BusLogic.h"

struct RenderCtx
{
    unsigned int shader = 0;
    int loc_uM = -1, loc_uV = -1, loc_uP = -1, loc_transparent = -1;
    int loc_uLightPos = -1, loc_uLightColor = -1, loc_uViewPos = -1, loc_uTint = -1;

    Shader* modelShader = nullptr;

    unsigned int VAO3D = 0;

    bool cullEnabled = true;

    glm::vec4 COL_WALL;
    glm::vec4 COL_FLOOR;
    glm::vec4 COL_FRAME;
    glm::vec4 COL_PANEL;
    glm::vec4 COL_DOOR;
    glm::vec4 COL_ROOF;
};

struct SceneState
{
    glm::vec3 camPos;
    glm::mat4 Vcam;
    glm::mat4 P;

    glm::vec3 busOffset;
    glm::vec3 lightPos;

    glm::mat4 PanelLocal;
};

namespace BusRender
{
    void DrawWorldAndBus(RenderCtx& ctx, const BusLogic& logic, SceneState& out);

    void DrawSteeringWheel(RenderCtx& ctx, const SceneState& s,
        Model& steeringWheel, float wheelSteerDeg, float wheelTiltDeg);

    void DrawActors(RenderCtx& ctx, const SceneState& s, const Model& controlModel, std::vector<Model>& people,
        const std::deque<Actor>& insideActors, bool hasMoving, const Actor& movingActor);
}
