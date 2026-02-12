#include "RouteData.h"

const int ROUTE_POINT_COUNT = 12;

const float route2D[ROUTE_POINT_COUNT * 2] = {
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

const int STOP_COUNT = 10;
const int stopIndices[STOP_COUNT] = { 0,1,2,4,5,7,8,9,10,11 };

glm::vec3 RoutePoint3D(int idx, float scale)
{
    float x = route2D[idx * 2 + 0] * scale;
    float z = route2D[idx * 2 + 1] * scale;
    return glm::vec3(x, 0.0f, z);
}

bool IsStopPoint(int routeIdx)
{
    for (int i = 0; i < STOP_COUNT; i++)
        if (stopIndices[i] == routeIdx) return true;
    return false;
}

int StopNumberForRouteIdx(int routeIdx)
{
    for (int i = 0; i < STOP_COUNT; i++)
        if (stopIndices[i] == routeIdx) return i;
    return -1;
}
