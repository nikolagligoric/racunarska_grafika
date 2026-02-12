#include "BusLogic.h"
#include "RouteData.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static float clamp01(float x) { return (x < 0.0f) ? 0.0f : (x > 1.0f ? 1.0f : x); }
static float smooth01(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

static constexpr float PASSENGER_MOVE_TIME = 1.6f;
static constexpr float CONTROL_MOVE_TIME = 2.4f;

BusLogic::BusLogic()
{
    reset(0.0);
}

void BusLogic::reset(double now)
{
    s.passengers = 0;
    s.passengerCount = 0;
    s.controlInside = false;
    s.totalFines = 0;

    s.currentRoutePoint = 0;
    s.travelT = 0.0f;
    s.atStop = true;
    s.stopStartTime = now;

    s.doorState = DoorState::OPEN;
    s.doorAction = DoorAction::NONE;
    s.doorActionTimer = 0.0f;

    s.busPos = RoutePoint3D(0);

    nextId = 1;
    inside.clear();
    movingActive = false;
    moving = Actor{};
}

void BusLogic::update(double now, double dt)
{
    updateMovingActor((float)dt);

    if (s.atStop)
    {
        s.doorState = DoorState::OPEN;

        processDoorAction(dt);

        if (now - s.stopStartTime >= 10.0)
            leaveStop();

        s.passengerCount = s.passengers;
        return;
    }

    s.doorState = DoorState::CLOSED;

    bool reached = moveAlongRoute((float)dt);
    if (reached && IsStopPoint(s.currentRoutePoint))
        arriveToStop(now);

    s.passengerCount = s.passengers;
}

void BusLogic::leaveStop()
{
    s.atStop = false;
    s.travelT = 0.0f;

    s.doorAction = DoorAction::NONE;
    s.doorActionTimer = 0.0f;
}

void BusLogic::arriveToStop(double now)
{
    s.atStop = true;
    s.stopStartTime = now;

    if (s.controlInside)
    {
        controlExitAndFine();

        if (!movingActive)
        {
            s.doorAction = DoorAction::EXITING;
            s.doorActionTimer = CONTROL_MOVE_TIME;

            startExitActor();
        }
    }
}

bool BusLogic::moveAlongRoute(float dt)
{
    int nextRoutePoint = (s.currentRoutePoint + 1) % ROUTE_POINT_COUNT;

    glm::vec3 c = RoutePoint3D(s.currentRoutePoint);
    glm::vec3 n = RoutePoint3D(nextRoutePoint);

    float len = glm::length(n - c);
    if (len < 1e-6f) len = 1e-6f;

    float worldSpeed = 1.25f;
    s.travelT += (worldSpeed / len) * dt;

    if (s.travelT >= 1.0f)
    {
        s.travelT = 0.0f;
        s.currentRoutePoint = nextRoutePoint;
        s.busPos = RoutePoint3D(s.currentRoutePoint);
        return true;
    }

    s.busPos = c + (n - c) * s.travelT;
    return false;
}

void BusLogic::processDoorAction(double dt)
{
    if (s.doorAction == DoorAction::NONE) return;

    s.doorActionTimer -= (float)dt;
    if (s.doorActionTimer <= 0.0f)
    {
        s.doorAction = DoorAction::NONE;
        s.doorActionTimer = 0.0f;
    }
}

glm::vec3 BusLogic::doorOutsidePos() const
{
    const float busWidth = 2.4f;
    const float busLength = 7.0f;
    const float busHeight = 0.98f;

    glm::vec3 busCenter = glm::vec3(0.0f, 1.10f, 2.30f);

    const float zFront = -busLength * 0.5f;
    const float zDoor0 = zFront;
    const float zDoor1 = zFront + 1.20f;
    const float zMid = 0.5f * (zDoor0 + zDoor1);

    float floorTopWorld = (busCenter.y - busHeight * 0.5f) + 0.10f * 0.5f;
    float yWorld = floorTopWorld + 0.25f;

    float wallX = busCenter.x + busWidth * 0.5f;

    float xWorld = wallX + 0.55f;

    return glm::vec3(xWorld, yWorld, busCenter.z + zMid);
}

glm::vec3 BusLogic::doorThresholdPos() const
{
    const float busWidth = 2.4f;
    const float busLength = 7.0f;
    const float busHeight = 0.98f;

    glm::vec3 busCenter = glm::vec3(0.0f, 1.10f, 2.30f);

    const float zFront = -busLength * 0.5f;
    const float zDoor0 = zFront;
    const float zDoor1 = zFront + 1.20f;
    const float zMid = 0.5f * (zDoor0 + zDoor1);

    float floorTopWorld = (busCenter.y - busHeight * 0.5f) + 0.10f * 0.5f;
    float yWorld = floorTopWorld + 0.25f;

    float wallX = busCenter.x + busWidth * 0.5f;

    float xWorld = wallX - 0.05f;

    return glm::vec3(xWorld, yWorld, busCenter.z + zMid);
}

glm::vec3 BusLogic::insideTargetPos() const
{
    const float busLength = 7.0f;
    const float busHeight = 0.98f;

    glm::vec3 busCenter = glm::vec3(0.0f, 1.10f, 2.30f);

    float floorTopWorld = (busCenter.y - busHeight * 0.5f) + 0.10f * 0.5f;
    float yWorld = floorTopWorld + 0.25f;

    float xWorld = busCenter.x + 0.20f;
    float zWorld = busCenter.z + (busLength * 0.20f);

    return glm::vec3(xWorld, yWorld, zWorld);
}

void BusLogic::startEnterActor(ActorType type)
{
    Actor a;
    a.id = nextId++;
    a.type = type;

    if (type == ActorType::Control)
    {
        a.modelIndex = 0;
    }
    else
    {
        a.modelIndex = (a.id - 1) % SKIN_COUNT;
    }


    a.anim = ActorAnim::Entering;

    a.startPos = doorOutsidePos();
    a.midPos = doorThresholdPos();
    a.endPos = insideTargetPos();
    a.useMid = true;

    a.pos = a.startPos;
    a.t = 0.0f;
    a.duration = (type == ActorType::Control) ? CONTROL_MOVE_TIME : PASSENGER_MOVE_TIME;

    moving = a;
    movingActive = true;
}


void BusLogic::startExitActor()
{
    if (inside.empty()) return;

    int idxControl = -1;
    for (int i = 0; i < (int)inside.size(); i++)
    {
        if (inside[i].type == ActorType::Control) { idxControl = i; break; }
    }

    Actor a;
    if (idxControl >= 0)
    {
        a = inside[idxControl];
        inside.erase(inside.begin() + idxControl);
    }
    else
    {
        a = inside.front();
        inside.pop_front();
    }

    a.anim = ActorAnim::Exiting;

    a.startPos = insideTargetPos();
    a.midPos = doorThresholdPos();
    a.endPos = doorOutsidePos();
    a.useMid = true;

    a.pos = a.startPos;
    a.t = 0.0f;
    a.duration = (a.type == ActorType::Control) ? CONTROL_MOVE_TIME : PASSENGER_MOVE_TIME;

    moving = a;
    movingActive = true;
}

void BusLogic::updateMovingActor(float dt)
{
    if (!movingActive) return;

    moving.t += (dt / std::max(0.001f, moving.duration));
    float t01 = clamp01(moving.t);

    if (!moving.useMid)
    {
        float k = smooth01(t01);
        moving.pos = moving.startPos + (moving.endPos - moving.startPos) * k;
    }
    else
    {
        if (t01 < 0.5f)
        {
            float k = smooth01(t01 / 0.5f);
            moving.pos = moving.startPos + (moving.midPos - moving.startPos) * k;
        }
        else
        {
            float k = smooth01((t01 - 0.5f) / 0.5f);
            moving.pos = moving.midPos + (moving.endPos - moving.midPos) * k;
        }
    }

    if (moving.t >= 1.0f)
    {
        if (moving.anim == ActorAnim::Entering)
        {
            moving.anim = ActorAnim::Inside;
            moving.pos = insideTargetPos();
            inside.push_back(moving);
        }

        movingActive = false;
        moving = Actor{};
    }
}

bool BusLogic::tryPassengerEnter()
{
    if (!s.atStop) return false;
    if (movingActive) return false;
    if (s.doorAction != DoorAction::NONE) return false;
    if (s.controlInside) return false;
    if (s.passengers >= 50) return false;

    s.passengers++;
    s.passengerCount = s.passengers;

    s.doorAction = DoorAction::ENTERING;
    s.doorActionTimer = PASSENGER_MOVE_TIME;

    startEnterActor(ActorType::Passenger);
    return true;
}

bool BusLogic::tryPassengerExit()
{
    if (!s.atStop) return false;
    if (movingActive) return false;
    if (s.doorAction != DoorAction::NONE) return false;
    if (s.controlInside) return false;
    if (s.passengers <= 0) return false;

    if (inside.empty()) return false;

    s.passengers--;
    s.passengerCount = s.passengers;

    s.doorAction = DoorAction::EXITING;
    s.doorActionTimer = PASSENGER_MOVE_TIME;

    startExitActor();
    return true;
}

bool BusLogic::tryControlEnter()
{
    if (!s.atStop) return false;
    if (movingActive) return false;
    if (s.doorAction != DoorAction::NONE) return false;
    if (s.controlInside) return false;
    if (s.passengers >= 50) return false;

    s.controlInside = true;
    s.passengers++;
    s.passengerCount = s.passengers;

    s.doorAction = DoorAction::ENTERING;
    s.doorActionTimer = CONTROL_MOVE_TIME;

    startEnterActor(ActorType::Control);
    return true;
}

void BusLogic::controlExitAndFine()
{
    int passengerOnly = s.passengers - 1;
    if (passengerOnly < 0) passengerOnly = 0;

    if (passengerOnly > 0)
    {
        int numFines = rand() % (passengerOnly + 1);
        s.totalFines += numFines;
    }

    if (s.passengers > 0) s.passengers--;
    s.passengerCount = s.passengers;
    s.controlInside = false;
}

