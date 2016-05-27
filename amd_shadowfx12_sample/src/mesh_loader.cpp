
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


/**
* @file      mesh_loader.cpp
* @brief     simple mesh loading function implemetation
*/

#include "mesh_loader.hpp"

#include <assimp/cimport.h>
#include <assimp/config.h>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

mesh_list load_mesh(std::string const& file)
{
    auto property_store = aiCreatePropertyStore();
    aiSetImportPropertyInteger(property_store, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    aiSetImportPropertyInteger(property_store, AI_CONFIG_PP_PTV_NORMALIZE, 1);

    auto scene = aiImportFileExWithProperties(file.c_str(),
        aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices | aiProcess_SplitLargeMeshes |
        aiProcess_PreTransformVertices, nullptr, property_store);

    if (scene == nullptr)
    {
        aiReleasePropertyStore(property_store);
        throw std::runtime_error{ "assimp import failed" };
    }

    mesh_list m(scene->mNumMeshes);
    for (size_t i = 0; i < m.size(); ++i)
    {
        if (scene->mMeshes[i]->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
        {
            continue;
        }

        auto mat_id = scene->mMeshes[i]->mMaterialIndex;
        if (mat_id < scene->mNumMaterials)
        {
            auto mtl = scene->mMaterials[mat_id];
            aiColor4D col{};
            if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &col) == AI_SUCCESS)
            {
                m[i].mat.ambient = tml::vec3{ col.r,  col.g, col.b };
            }
            if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &col) == AI_SUCCESS)
            {
                m[i].mat.diffuse = tml::vec3{ col.r,  col.g, col.b };
            }
            if (aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &col) == AI_SUCCESS)
            {
                m[i].mat.specular = tml::vec3{ col.r,  col.g, col.b };
            }
            float shininess = 0.f;
            if (aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS)
            {
                m[i].mat.shininess = shininess;
            }
        }

        m[i].vbo.ib.resize(scene->mMeshes[i]->mNumFaces * 3);
        for (unsigned j = 0; j < scene->mMeshes[i]->mNumFaces; ++j)
        {
            for (size_t k = 0; k < 3; ++k)
            {
                m[i].vbo.ib[j * 3 + k] = scene->mMeshes[i]->mFaces[j].mIndices[k];
            }
        }

        m[i].vbo.vb.resize(scene->mMeshes[i]->mNumVertices);
        assert(scene->mMeshes[i]->mVertices != nullptr);
        for (unsigned j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
        {
            m[i].vbo.vb[j].position.x = scene->mMeshes[i]->mVertices[j].x;
            m[i].vbo.vb[j].position.y = scene->mMeshes[i]->mVertices[j].y;
            m[i].vbo.vb[j].position.z = scene->mMeshes[i]->mVertices[j].z;
        }

        if (scene->mMeshes[i]->mNormals)
        {
            for (unsigned j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
            {
                m[i].vbo.vb[j].normal.x = scene->mMeshes[i]->mNormals[j].x;
                m[i].vbo.vb[j].normal.y = scene->mMeshes[i]->mNormals[j].y;
                m[i].vbo.vb[j].normal.z = scene->mMeshes[i]->mNormals[j].z;
            }
        }

        if (scene->mMeshes[i]->mTangents)
        {
            for (unsigned j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
            {
                m[i].vbo.vb[j].tangent.x = scene->mMeshes[i]->mTangents[j].x;
                m[i].vbo.vb[j].tangent.y = scene->mMeshes[i]->mTangents[j].y;
                m[i].vbo.vb[j].tangent.z = scene->mMeshes[i]->mTangents[j].z;
            }
        }
        if (scene->mMeshes[i]->mTextureCoords[0])
        {
            for (unsigned j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
            {
                m[i].vbo.vb[j].uv.x = scene->mMeshes[i]->mTextureCoords[0][j].x;
                m[i].vbo.vb[j].uv.y = scene->mMeshes[i]->mTextureCoords[0][j].y;
            }
        }

    }

    aiReleaseImport(scene);
    aiReleasePropertyStore(property_store);

    return m;
}

