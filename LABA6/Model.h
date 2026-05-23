#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "Shader.h"

class Model {
public:
    Model(const char* path) {
        loadModel(path);
        std::cout << "Loaded " << meshes.size() << " meshes" << std::endl;
    }

    void Draw(Shader& shader,
        const glm::mat4& parentMatrix,
        float OX1_Angle,
        float OX2_Angle,
        float OX3_Angle,
        float OX1_OffsetX, float OX1_OffsetY, float OX1_OffsetZ,
        float OX2_OffsetX, float OX2_OffsetY, float OX2_OffsetZ,
        float OX3_OffsetX, float OX3_OffsetY, float OX3_OffsetZ) {

        // OX1 вращение вокруг Y
        glm::mat4 OX1_Local = rotateAroundPoint(OX1_Angle,
            glm::vec3(0.0f, 1.0f, 0.0f),
            OX1_OffsetX, OX1_OffsetY, OX1_OffsetZ);
        glm::mat4 OX1_Matrix = parentMatrix * OX1_Local;

        // OX2 вращение вокруг Z наследует поворот OX1
        glm::mat4 OX2_Local = rotateAroundPoint(OX2_Angle,
            glm::vec3(0.0f, 0.0f, 1.0f),
            OX2_OffsetX, OX2_OffsetY, OX2_OffsetZ);
        glm::mat4 OX2_Matrix = OX1_Matrix * OX2_Local;

        // OX3 вращение вокруг Z наследует поворот OX2
        glm::mat4 OX3_Local = rotateAroundPoint(OX3_Angle,
            glm::vec3(0.0f, 0.0f, 1.0f),
            OX3_OffsetX, OX3_OffsetY, OX3_OffsetZ);
        glm::mat4 OX3_Matrix = OX2_Matrix * OX3_Local;

        // ќтрисовка 4 мешей модели
        for (unsigned int i = 0; i < meshes.size(); i++) {
            glm::mat4 modelMatrix;

            switch (i) {
            case 0:  // Ќеподвижное основание
                modelMatrix = parentMatrix;
                break;
            case 1:  // OX1
                modelMatrix = OX1_Matrix;
                break;
            case 2:  // OX3
                modelMatrix = OX3_Matrix;
                break;
            case 3:  // OX2
                modelMatrix = OX2_Matrix;
                break;
            default:
                modelMatrix = glm::mat4(1.0f);
            }

            //  орректное преобразование нормалей дл€ освещени€
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
            shader.setMat4("model", modelMatrix);
            shader.setMat3("normalMatrix", normalMatrix);

            meshes[i].Draw();
        }
    }

private:
    std::vector<Mesh> meshes;
    std::string directory;

    // ѕоворот вокруг произвольной точки
    glm::mat4 rotateAroundPoint(float angle, glm::vec3 axis, float x, float y, float z) {
        glm::mat4 matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, glm::vec3(x, y, z));
        matrix = glm::rotate(matrix, glm::radians(angle), axis);
        matrix = glm::translate(matrix, glm::vec3(-x, -y, -z));
        return matrix;
    }

    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
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

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            }
            else {
                vertex.Normal = glm::vec3(0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices);
    }
};

#endif