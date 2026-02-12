#include "BusRender.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace BusRender
{
    static float YawFromDir(const glm::vec3& dir)
    {
        glm::vec3 d = dir; d.y = 0.0f;
        float len = glm::length(d);
        if (len < 1e-6f) return 0.0f;
        d /= len;
        float yawRad = atan2(d.x, -d.z);
        return glm::degrees(yawRad);
    }

    static void ApplyTint(RenderCtx& ctx, const glm::vec4& c)
    {
        glUniform4f(ctx.loc_uTint, c.r, c.g, c.b, c.a);
    }

    static void DrawCube(RenderCtx& ctx, const glm::mat4& Mworld)
    {
        glUniformMatrix4fv(ctx.loc_uM, 1, GL_FALSE, glm::value_ptr(Mworld));
        glBindVertexArray(ctx.VAO3D);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    static glm::mat4 BoxTRS(const glm::vec3& c, const glm::vec3& s)
    {
        return glm::scale(glm::translate(glm::mat4(1.0f), c), s);
    }

    void DrawWorldAndBus(RenderCtx& ctx, const BusLogic& logic, SceneState& out)
    {
        const auto& st = logic.state();

        glUseProgram(ctx.shader);

        glUniform3f(ctx.loc_uLightPos, out.lightPos.x, out.lightPos.y, out.lightPos.z);
        glUniform3f(ctx.loc_uLightColor, 1.0f, 0.95f, 0.85f);
        glUniform3f(ctx.loc_uViewPos, out.camPos.x, out.camPos.y, out.camPos.z);

        glUniformMatrix4fv(ctx.loc_uV, 1, GL_FALSE, glm::value_ptr(out.Vcam));
        glUniformMatrix4fv(ctx.loc_uP, 1, GL_FALSE, glm::value_ptr(out.P));

        auto drawCubeLocal = [&](const glm::mat4& Mlocal)
            {
                glm::mat4 Mworld = glm::translate(glm::mat4(1.0f), out.busOffset) * Mlocal;
                DrawCube(ctx, Mworld);
            };

        glUniform1i(ctx.loc_transparent, 0);

        glm::mat4 Lamp(1.0f);
        Lamp = glm::translate(Lamp, out.lightPos);
        Lamp = glm::scale(Lamp, glm::vec3(0.12f, 0.06f, 0.12f));
        ApplyTint(ctx, glm::vec4(1.0f, 1.0f, 0.9f, 1.0f));
        DrawCube(ctx, Lamp);

        glm::vec3 panelCenter = glm::vec3(-0.25f, 0.67f, -0.62f);
        glm::vec3 panelSize = glm::vec3(1.5f, 0.42f, 0.48f);
        glm::vec3 panelHinge = panelCenter + glm::vec3(0.0f, 0.5f * panelSize.y, -0.5f * panelSize.z);

        glm::mat4 Panel(1.0f);
        Panel = glm::translate(Panel, panelHinge);
        Panel = glm::rotate(Panel, glm::radians(-12.0f), glm::vec3(1, 0, 0));
        Panel = glm::translate(Panel, -panelHinge);
        Panel = glm::translate(Panel, panelCenter);
        Panel = glm::scale(Panel, panelSize);

        out.PanelLocal = Panel;

        ApplyTint(ctx, ctx.COL_PANEL);
        drawCubeLocal(Panel);

        glm::vec3 frameC = glm::vec3(0.0f, 1.15f, -1.20f);
        glm::vec3 outerS = glm::vec3(2.4f, 0.95f, 0.08f);
        float t = 0.12f;

        ApplyTint(ctx, ctx.COL_FRAME);

        glm::mat4 Top = BoxTRS(frameC + glm::vec3(0.0f, outerS.y * 0.5f - t * 0.5f, 0.0f), glm::vec3(outerS.x, t, outerS.z));
        glm::mat4 Bottom = BoxTRS(frameC + glm::vec3(0.0f, -outerS.y * 0.5f + t * 0.5f, 0.0f), glm::vec3(outerS.x, t, outerS.z));

        float sideH = outerS.y - 2.0f * t;
        glm::mat4 Left = BoxTRS(frameC + glm::vec3(-outerS.x * 0.5f + t * 0.5f, 0.0f, 0.0f), glm::vec3(t, sideH, outerS.z));
        glm::mat4 Right = BoxTRS(frameC + glm::vec3(outerS.x * 0.5f - t * 0.5f, 0.0f, 0.0f), glm::vec3(t, sideH, outerS.z));

        drawCubeLocal(Top);
        drawCubeLocal(Bottom);
        drawCubeLocal(Left);
        drawCubeLocal(Right);

        glm::vec3 glassC = glm::vec3(0.0f, 1.15f, -1.15f);
        glm::vec3 glassS = glm::vec3(2.3f, 0.88f, 0.03f);
        glm::mat4 Glass = BoxTRS(glassC, glassS);

        glUniform1i(ctx.loc_transparent, 1);
        ApplyTint(ctx, glm::vec4(0.25f, 0.28f, 0.33f, 0.28f));
        glDepthMask(GL_FALSE);
        GLboolean wasCullGlass = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);
        drawCubeLocal(Glass);
        if (wasCullGlass && ctx.cullEnabled) glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glUniform1i(ctx.loc_transparent, 0);

        const float busWidth = 2.4f;
        const float busHeight = 0.98f;
        const float busLength = 7.0f;
        glm::vec3 busCenter = glm::vec3(0.0f, 1.10f, 2.30f);

        auto Box = [&](glm::vec3 c, glm::vec3 s, const glm::vec4& tint)
            {
                glm::mat4 M = BoxTRS(c, s);
                ApplyTint(ctx, tint);
                drawCubeLocal(M);
            };

        Box(busCenter + glm::vec3(0.0f, -busHeight * 0.5f, 0.0f), glm::vec3(busWidth, 0.10f, busLength), ctx.COL_FLOOR);
        Box(busCenter + glm::vec3(0.0f, +busHeight * 0.5f, 0.0f), glm::vec3(busWidth, 0.10f, busLength), ctx.COL_ROOF);
        Box(busCenter + glm::vec3(-busWidth * 0.5f, 0.0f, 0.0f), glm::vec3(0.08f, busHeight, busLength), ctx.COL_WALL);

        {
            const float zFront = -busLength * 0.5f;
            const float zBack = +busLength * 0.5f;
            const float zDoor0 = zFront;
            const float zDoor1 = zFront + 1.20f;
            const float wallT = 0.08f;

            auto RightWallSegment = [&](float z0, float z1)
                {
                    float segLen = (z1 - z0);
                    if (segLen <= 0.001f) return;

                    glm::vec3 c = busCenter + glm::vec3(+busWidth * 0.5f, 0.0f, (z0 + z1) * 0.5f);
                    glm::vec3 s = glm::vec3(wallT, busHeight, segLen);
                    Box(c, s, ctx.COL_WALL);
                };

            RightWallSegment(zDoor1, zBack);

            const float wallDoorT = 0.06f;
            const float doorLen = (zDoor1 - zDoor0);

            glm::vec3 doorCenterClosed =
                busCenter + glm::vec3(+busWidth * 0.5f - wallDoorT * 0.5f, 0.0f, (zDoor0 + zDoor1) * 0.5f);

            glm::vec3 hinge =
                busCenter + glm::vec3(+busWidth * 0.5f, 0.0f, zDoor0);

            float doorAnim = st.atStop ? 1.0f : 0.0f;
            float angle = glm::radians(80.0f * doorAnim);

            glm::mat4 M(1.0f);
            M = glm::translate(M, hinge);
            M = glm::rotate(M, angle, glm::vec3(0, 1, 0));
            M = glm::translate(M, doorCenterClosed - hinge);
            M = glm::scale(M, glm::vec3(wallDoorT, busHeight, doorLen));

            GLboolean wasCullDoor = glIsEnabled(GL_CULL_FACE);
            glDisable(GL_CULL_FACE);
            ApplyTint(ctx, ctx.COL_DOOR);
            drawCubeLocal(M);
            if (wasCullDoor && ctx.cullEnabled) glEnable(GL_CULL_FACE);
        }

        {
            glm::vec3 worldOrigin = out.busOffset + glm::vec3(0.0f, 0.55f, -2.0f);

            auto boxWorld = [&](glm::vec3 c, glm::vec3 s, glm::vec4 tint)
                {
                    glm::mat4 M(1.0f);
                    M = glm::translate(M, c);
                    M = glm::scale(M, s);
                    ApplyTint(ctx, tint);
                    DrawCube(ctx, M);
                };

            boxWorld(worldOrigin + glm::vec3(0.0f, 0.0f, -40.0f), glm::vec3(6.0f, 0.05f, 80.0f), glm::vec4(0.22f, 0.22f, 0.24f, 1.0f));

            for (int i = 0; i < 30; i++)
            {
                float z = -3.0f - i * 2.8f;
                boxWorld(worldOrigin + glm::vec3(0.0f, 0.03f, z), glm::vec3(0.15f, 0.02f, 1.2f), glm::vec4(0.95f, 0.95f, 0.95f, 1.0f));
            }

            boxWorld(worldOrigin + glm::vec3(-2.7f, 0.03f, -40.0f), glm::vec3(0.10f, 0.02f, 80.0f), glm::vec4(0.90f, 0.90f, 0.90f, 1.0f));
            boxWorld(worldOrigin + glm::vec3(+2.7f, 0.03f, -40.0f), glm::vec3(0.10f, 0.02f, 80.0f), glm::vec4(0.90f, 0.90f, 0.90f, 1.0f));

            for (int i = 0; i < 18; i++)
            {
                float z = -6.0f - i * 4.5f;
                float h = 1.5f + 0.25f * (i % 6);
                boxWorld(worldOrigin + glm::vec3(-8.0f, h * 0.5f, z), glm::vec3(4.0f, h, 3.0f), glm::vec4(0.35f, 0.35f, 0.38f, 1.0f));
            }
            for (int i = 0; i < 18; i++)
            {
                float z = -6.0f - i * 4.5f;
                float h = 1.2f + 0.30f * ((i + 2) % 6);
                boxWorld(worldOrigin + glm::vec3(+8.0f, h * 0.5f, z), glm::vec3(4.5f, h, 3.2f), glm::vec4(0.38f, 0.36f, 0.34f, 1.0f));
            }
        }
    }

    void DrawSteeringWheel(RenderCtx& ctx, const SceneState& s, Model& steeringWheel, float wheelSteerDeg, float wheelTiltDeg)
    {
        Shader& sh = *ctx.modelShader;
        sh.use();

        sh.setVec3("uLightPos", s.lightPos.x, s.lightPos.y, s.lightPos.z);
        sh.setVec3("uLightColor", 1.0f, 0.95f, 0.85f);
        sh.setVec3("uViewPos", s.camPos.x, s.camPos.y, s.camPos.z);
        sh.setMat4("uV", s.Vcam);
        sh.setMat4("uP", s.P);

        glm::vec3 wheelCenter = glm::vec3(-0.55f, 0.94f, -0.24f);
        const float WHEEL_SCALE = 0.30f;

        glm::mat4 M(1.0f);
        M = glm::translate(M, s.busOffset + wheelCenter);
        M = glm::rotate(M, glm::radians(+wheelTiltDeg), glm::vec3(1, 0, 0));
        M = glm::rotate(M, glm::radians(wheelSteerDeg), glm::vec3(0, 1, 0));
        M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        M = glm::scale(M, glm::vec3(WHEEL_SCALE));

        sh.setMat4("uM", M);

        GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);
        steeringWheel.Draw(sh);
        if (wasCull && ctx.cullEnabled) glEnable(GL_CULL_FACE);

        glUseProgram(ctx.shader);
    }

    void DrawActors(RenderCtx& ctx, const SceneState& s, std::vector<Model>& people,
        const std::deque<Actor>& insideActors, bool hasMoving, const Actor& movingActor)
    {
        Shader& sh = *ctx.modelShader;
        sh.use();

        sh.setVec3("uLightPos", s.lightPos.x, s.lightPos.y, s.lightPos.z);
        sh.setVec3("uLightColor", 1.0f, 0.95f, 0.85f);
        sh.setVec3("uViewPos", s.camPos.x, s.camPos.y, s.camPos.z);
        sh.setMat4("uV", s.Vcam);
        sh.setMat4("uP", s.P);

        const float CHAR_SCALE = 0.002f;
        const float CHAR_Y_OFF = -0.2f;
        const glm::vec3 CHAR_PIVOT(-0.1f, 0.0f, 0.0f);

        auto drawOne = [&](const Actor& a, bool isMoving)
            {
                float yaw = 180.0f;
                if (isMoving)
                {
                    glm::vec3 dir = a.endPos - a.startPos;
                    if (a.anim == ActorAnim::Exiting) dir = -dir;
                    yaw = YawFromDir(dir);
                    yaw += 90.0f;
                    if (a.anim == ActorAnim::Exiting) yaw += 180.0f;
                }

                glm::mat4 M(1.0f);
                M = glm::translate(M, a.pos + glm::vec3(0.0f, CHAR_Y_OFF, 0.0f));
                M = glm::rotate(M, glm::radians(yaw), glm::vec3(0, 1, 0));
                M = glm::translate(M, CHAR_PIVOT);
                M = glm::scale(M, glm::vec3(CHAR_SCALE));

                sh.setMat4("uM", M);

                GLboolean wasCull = glIsEnabled(GL_CULL_FACE);
                glDisable(GL_CULL_FACE);

                int idx = a.modelIndex;
                if (idx < 0 || idx >= (int)people.size()) idx = 0;
                people[idx].Draw(sh);

                if (wasCull && ctx.cullEnabled) glEnable(GL_CULL_FACE);
            };

        for (const auto& a : insideActors) drawOne(a, false);
        if (hasMoving) drawOne(movingActor, true);

        glUseProgram(ctx.shader);
    }
}
