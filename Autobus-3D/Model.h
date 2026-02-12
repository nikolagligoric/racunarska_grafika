#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 nor;
};

struct Mesh {
    unsigned int vao = 0, vbo = 0;
    unsigned int count = 0;

    void draw() const;
};

class Model {
public:
    bool load(const std::string& path);
    void draw() const;

private:
    std::vector<Mesh> meshes;
};
