#ifndef MODEL_H
#define MODEL_H

#include "stb_image.h"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

static const string KENNEY_SKIN_DIR = "res/kenney_animated-characters-1/Skins";
static const string KENNEY_DEFAULT_SKIN = "survivorMaleB.png";


static inline string Basename(const string& p)
{
    size_t s = p.find_last_of("/\\");
    return (s == string::npos) ? p : p.substr(s + 1);
}

static inline bool FileExists(const string& p)
{
    std::ifstream f(p, std::ios::binary);
    return (bool)f;
}

static inline GLenum CompToFormat(int comp)
{
    if (comp == 1) return GL_RED;
    if (comp == 3) return GL_RGB;
    return GL_RGBA;
}

static inline unsigned int TextureFromEmbedded(const aiTexture* tex)
{
    if (!tex) return 0;

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (tex->mHeight == 0)
    {
        const unsigned char* data = reinterpret_cast<const unsigned char*>(tex->pcData);
        int w = 0, h = 0, comp = 0;

        unsigned char* image = stbi_load_from_memory(
            data,
            (int)tex->mWidth,
            &w, &h, &comp,
            0
        );

        if (!image || w <= 0 || h <= 0)
        {
            std::cout << "[ASSIMP] Embedded texture decode failed.\n";
            if (image) stbi_image_free(image);
            glDeleteTextures(1, &textureID);
            return 0;
        }

        GLenum format = CompToFormat(comp);

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
    }
    else
    {
        int w = (int)tex->mWidth;
        int h = (int)tex->mHeight;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex->pcData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

class Model
{
public:
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    Model(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

private:
    void loadModel(string const& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_PreTransformVertices
        );

        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        size_t slash = path.find_last_of("/\\");
        directory = (slash == string::npos) ? "" : path.substr(0, slash);

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    unsigned int TryLoadTextureSmart(const char* assimpPathCStr)
    {
        string assimpPath = string(assimpPathCStr);
        string base = Basename(assimpPath);

        if (!directory.empty())
        {
            string p1 = directory + "/" + assimpPath;
            if (FileExists(p1)) return TextureFromFile(assimpPathCStr, directory);
        }

        if (!directory.empty())
        {
            string p2 = directory + "/" + base;
            if (FileExists(p2)) return TextureFromFile(base.c_str(), directory);
        }

        if (!KENNEY_SKIN_DIR.empty())
        {
            string p3 = KENNEY_SKIN_DIR + "/" + base;
            if (FileExists(p3)) return TextureFromFile(base.c_str(), KENNEY_SKIN_DIR);
        }

        return TextureFromFile(assimpPathCStr, directory);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 v;

            v.x = mesh->mVertices[i].x;
            v.y = mesh->mVertices[i].y;
            v.z = mesh->mVertices[i].z;
            vertex.Position = v;

            if (mesh->HasNormals())
            {
                v.x = mesh->mNormals[i].x;
                v.y = mesh->mNormals[i].y;
                v.z = mesh->mNormals[i].z;
                vertex.Normal = v;
            }

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 uv;
                uv.x = mesh->mTextureCoords[0][i].x;
                uv.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = uv;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "uDiffMap", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        if (diffuseMaps.empty())
        {
            Texture t;
            t.id = TextureFromFile(KENNEY_DEFAULT_SKIN.c_str(), KENNEY_SKIN_DIR);
            t.type = "uDiffMap";
            t.path = "KENNEY_FALLBACK";
            if (t.id != 0)
            {
                textures.push_back(t);
                textures_loaded.push_back(t);
            }
        }

        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "uSpecMap", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene)
    {
        vector<Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (skip) continue;

            Texture texture;
            texture.type = typeName;
            texture.path = str.C_Str();

            if (str.C_Str()[0] == '*' && scene && scene->mNumTextures > 0)
            {
                int texIndex = std::atoi(str.C_Str() + 1);
                if (texIndex >= 0 && (unsigned)texIndex < scene->mNumTextures)
                {
                    texture.id = TextureFromEmbedded(scene->mTextures[texIndex]);
                }
                else
                {
                    texture.id = 0;
                }
            }
            else
            {
                texture.id = TryLoadTextureSmart(str.C_Str());
            }

            if (texture.id != 0)
            {
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }

        return textures;
    }
};

inline unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);

    bool isAbs = (!filename.empty() && (filename[0] == '/' || filename[0] == '\\')) ||
        (filename.size() > 1 && filename[1] == ':');

    if (!isAbs)
    {
        if (!directory.empty())
            filename = directory + "/" + filename;
    }

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width = 0, height = 0, nrComponents = 0;

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);

        if (textureID)
        {
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }
    }

    return textureID;
}


#endif
