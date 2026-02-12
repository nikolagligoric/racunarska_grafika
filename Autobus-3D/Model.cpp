#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, count);
    glBindVertexArray(0);
}

bool Model::load(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || !scene->HasMeshes())
        return false;

    for (unsigned i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* m = scene->mMeshes[i];
        std::vector<Vertex> verts;

        for (unsigned v = 0; v < m->mNumVertices; v++)
        {
            Vertex vert;
            vert.pos = { m->mVertices[v].x, m->mVertices[v].y, m->mVertices[v].z };
            vert.nor = { m->mNormals[v].x,  m->mNormals[v].y,  m->mNormals[v].z };
            verts.push_back(vert);
        }

        Mesh mesh;
        mesh.count = (unsigned)verts.size();

        glGenVertexArrays(1, &mesh.vao);
        glGenBuffers(1, &mesh.vbo);

        glBindVertexArray(mesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        meshes.push_back(mesh);
    }

    return true;
}

void Model::draw() const {
    for (auto& m : meshes)
        m.draw();
}
