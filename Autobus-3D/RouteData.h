#pragma once
#include <vector>
#include <glm/glm.hpp>

extern const int STOP_COUNT;
extern const int ROUTE_POINT_COUNT;

extern const float route2D[];
extern const int stopIndices[];

glm::vec3 RoutePoint3D(int idx, float scale = 5.0f);
bool IsStopPoint(int routeIdx);
int StopNumberForRouteIdx(int routeIdx);
