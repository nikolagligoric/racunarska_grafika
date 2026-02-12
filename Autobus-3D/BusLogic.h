#pragma once
#include <glm/glm.hpp>
#include <deque>

enum class DoorState { CLOSED, OPENING, OPEN, CLOSING };
enum class DoorAction { NONE, ENTERING, EXITING };

enum class ActorType { Passenger, Control };
enum class ActorAnim { None, Entering, Inside, Exiting };

struct Actor
{
    int id = 0;
    ActorType type = ActorType::Passenger;

    int modelIndex = 0;
    ActorAnim anim = ActorAnim::None;

    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 startPos = glm::vec3(0.0f);

    glm::vec3 midPos = glm::vec3(0.0f);
    bool useMid = false;

    glm::vec3 endPos = glm::vec3(0.0f);

    float t = 0.0f;
    float duration = 1.0f; 
};

struct BusState
{
    int passengers = 0;
    int passengerCount = 0;
    bool controlInside = false;
    int totalFines = 0;

    int currentRoutePoint = 0;
    float travelT = 0.0f;
    bool atStop = true;
    double stopStartTime = 0.0;

    DoorState doorState = DoorState::OPEN;
    DoorAction doorAction = DoorAction::NONE;
    float doorActionTimer = 0.0f;

    glm::vec3 busPos = glm::vec3(0);
};

class BusLogic
{
public:
    BusLogic();

    void reset(double now);
    void update(double now, double dt);

    bool tryPassengerEnter();
    bool tryPassengerExit();
    bool tryControlEnter();

    const BusState& state() const { return s; }

    const std::deque<Actor>& insideActors() const { return inside; }
    bool hasMovingActor() const { return movingActive; }
    const Actor& movingActor() const { return moving; }

    static constexpr int SKIN_COUNT = 18;

private:
    BusState s;

    int nextId = 1;
    std::deque<Actor> inside;

    bool movingActive = false;
    Actor moving;

    void arriveToStop(double now);
    void leaveStop();

    void processDoorAction(double dt);
    bool moveAlongRoute(float dt);

    void controlExitAndFine();

    void startEnterActor(ActorType type);
    void startExitActor();
    void updateMovingActor(float dt);

    glm::vec3 doorOutsidePos() const;
    glm::vec3 doorThresholdPos() const;
    glm::vec3 insideTargetPos() const;
};
