#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include "shader.h"
#include "stb_image.h"
#include "camera.h"
#include "mesh.h"
#include "cage.h"
#include "bbox.h"

using namespace std;
using namespace glm;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

enum DrawMode {
    OBJECT,
    PHYSICS,
};

class Model {
    public:
        Cage cage;

        Model(string path, ModelShader shaders, bool isRigid = true)
        {
            this->shaders = shaders;
            this->isRigid = isRigid;
            loadModel(path);
        }

        void Draw(DrawMode mode) {
            if (mode == OBJECT) {
                for (unsigned int i = 0; i < meshes.size(); i++) {
                    shaders.matShader->use();
                    meshes[i].Draw(*(shaders.matShader));
                }
            }
            if (mode == PHYSICS) {
                cage.Draw(*(shaders.ptMassShader), *(shaders.springShader));
            }
        }

        unsigned int numVertices() {
            unsigned int sum = 0;
            for (Mesh m : meshes) {
                sum += m.vertices.size();
            }
            return sum;
        }

    private:
        // model data
        vector<Mesh> meshes;
        string directory;
        vector<Texture> textures_loaded;
        bool isRigid = true;
        ModelShader shaders;
        unsigned int idxMeshVertices;

        void loadModel(string path) {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path,
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenBoundingBoxes);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
                return;
            }
            directory = path.substr(0, path.find_last_of('/'));
            processNode(scene->mRootNode, scene);
            if (!isRigid) {
                processCage();
            }
        }

        void processNode(aiNode* node, const aiScene* scene) {
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(processMesh(mesh, scene));
            }
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], scene);
            }
        }

        void processCage() {
            for (const Mesh& mesh : meshes) {
                vec3 minBox = mesh.bbox.min;
                vec3 maxBox = mesh.bbox.max;
                vec3 box = mesh.bbox.dim;
                cout << "dim: " << box.x << ", " << box.y << ", " << box.z << endl;

                float len = std::max(box.x, box.y);
                len = ceil(std::max(len, box.z));
                cout << "len " << len << endl;

                cage = Cube(len, 2, vec3(0.0f, 5.0f, 0.0f));

                Cage newCage;
                map<int, int> idxMap; // maps old idx -> new idx
                newCage.pos = vec3(0.0f, 5.0f, 0.0f);
                for (int i = 0; i < cage.pts.size(); i++) {
                    PointMass* p = &cage.pts[i];
                    if (inMesh(p->Position, mesh)) {
                        newCage.pts.push_back(*p);
                        int newIdx = newCage.pts.size() - 1;
                        idxMap[i] = newIdx;
                        
                    }
                }
                for (const Spring& s : cage.springs) {
                    int v0_old = s.v0;
                    int v1_old = s.v1;

                    // check if both old idx were mapped (i.e. pt masses still exist)
                    if (idxMap.count(v0_old) && idxMap.count(v1_old)) {
                        Spring newSpring = Spring(idxMap[v0_old], idxMap[v1_old], s.type, s.restLength);
                        newCage.springs.push_back(newSpring);
                    }
                }

                idxMeshVertices = newCage.pts.size();
                for (const Vertex& v : mesh.vertices) {
                    newCage.pts.push_back(PointMass(v.Position, 1.0f));
                    int newIdx = newCage.pts.size() - 1;

                    // connect new mesh point to n closest cage vertices
                    auto closest = min_element(newCage.pts.begin(), newCage.pts.begin() + idxMeshVertices,
                        [v](const PointMass& a, const PointMass& b) {
                            return length(v.Position - a.Position) < length(v.Position - b.Position);
                        });
                    int closestIdx = closest - newCage.pts.begin();

                    float dist = length(v.Position - closest->Position);

                    Spring newSpring = Spring(newIdx, closestIdx, SURFACE, dist);
                    newCage.springs.push_back(newSpring);
                }
                for (int i = 0; i < mesh.indices.size() - 1; i += 2) {
                    int v0 = mesh.indices[i] + idxMeshVertices;
                    int v1 = mesh.indices[i + 1] + idxMeshVertices;
                    float dist = glm::length(newCage.pts[v0].Position - newCage.pts[v1].Position);

                    Spring newSpring = Spring(v0, v1, SURFACE, dist);
                    newCage.springs.push_back(newSpring);
                }

                cage = newCage;

                

                
                cage.refreshMesh();
            }
        }

        bool inMesh(const vec3& p, const Mesh& mesh) {
            Vertex closest = *min_element(mesh.vertices.begin(), mesh.vertices.end(), [p](const Vertex& a, const Vertex& b) {
                return length(a.Position - p) < length(b.Position - p);
                });
            //cout << "closest mesh vertex to (" << p.x << ", " << p.y << ", " << p.z << ") is ("
                //<< closest.Position.x << ", " << closest.Position.y << ", " << closest.Position.z << ") | ";

            vec3 dir = normalize(closest.Position - p);
            if (dot(closest.Normal, dir) < 0) {
                //cout << "not in mesh" << endl;

                return false;
            }

            //cout << "in mesh" << endl;

            return true;
        }

        Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
            vector<Vertex> vertices;
            vector<unsigned int> indices;
            vector<Texture> textures;

            
            aiAABB bb = mesh->mAABB;
            vec3 min(bb.mMin.x, bb.mMin.y, bb.mMin.z);
            vec3 max(bb.mMax.x, bb.mMax.y, bb.mMax.z);

            BBox b(min, max);

            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                Vertex vertex;
                vec3 v;
                v.x = mesh->mVertices[i].x;
                v.y = mesh->mVertices[i].y;
                v.z = mesh->mVertices[i].z;
                vertex.Position = v;

                v.x = mesh->mNormals[i].x;
                v.y = mesh->mNormals[i].y;
                v.z = mesh->mNormals[i].z;
                vertex.Normal = v;

                if (mesh->mTextureCoords[0]) {
                    vec2 v;
                    v.x = mesh->mTextureCoords[0][i].x;
                    v.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = v;
                }
                else {
                    vertex.TexCoords = vec2(0.0f, 0.0f);
                }

                vertices.push_back(vertex);
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }

            if (mesh->mMaterialIndex >= 0) {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                
                vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            }

            return Mesh(vertices, indices, textures, b);
        }


        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
            vector<Texture> textures;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str;
                mat->GetTexture(type, i, &str);
                bool skip = false;
                for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                    if (strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                        textures.push_back(textures_loaded[j]);
                        skip = true;
                        break;
                    }
                }
                if (!skip) {
                    Texture texture;
                    texture.id = TextureFromFile(str.C_Str(), directory);
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);
                }
            }
            return textures;
        }
};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

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
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif