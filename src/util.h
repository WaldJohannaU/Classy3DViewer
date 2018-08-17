/*******************************************************
 * Copyright (c) 2017, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#ifndef _H_UTIL_
#define _H_UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#include <nanogui/opengl.h>

// Includes for GLTexture
#include <cstdint>
#include <memory>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing fla

/*bool loadAssImp(const char* path,
                std::vector<unsigned int>& indices,
                std::vector<float>& vertices,
                std::vector<float>& uvs,
                std::vector<float>& normals) {
    Assimp::Importer importer;
    
    std::ifstream f(path);
    if (!f.good())
        return false;
    
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
    if (!scene) {
        std::cout << importer.GetErrorString() << std::endl;
        getchar();
        return false;
    }
    const aiMesh* mesh = scene->mMeshes[0];
    
    // Fill vertices positions
    vertices.reserve(mesh->mNumVertices*3);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        aiVector3D pos = mesh->mVertices[i];
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);
    }
        
    // Fill vertices texture coordinates
    uvs.reserve(mesh->mNumVertices);
    if (mesh->mTextureCoords[0] != nullptr) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
            uvs.push_back(UVW.x);
            uvs.push_back(UVW.y);
        }
    }

    // Fill vertices normals
    normals.reserve(mesh->mNumVertices);
    for (unsigned int i=0; i < mesh->mNumVertices; i++){
        aiVector3D n = mesh->mNormals[i];
        normals.push_back(n.x);
        normals.push_back(n.y);
        normals.push_back(n.z);
    }
    
    // Fill face indices
    indices.reserve(3*mesh->mNumFaces);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++){
        // Assume the model has only triangles.
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
    return true;
}*/

/*bool BindCVMat2GLTexture(const cv::Mat& image, GLuint& imageTexture, bool conv) {
    if (!image.empty()) {
        glDeleteTextures(1, &imageTexture);
        glGenTextures(1, &imageTexture);
        glBindTexture(GL_TEXTURE_2D, imageTexture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,                   // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGB,              // Internal colour format to convert to
                     image.cols,
                     image.rows,
                     0,                   // Border width in pixels (can either be 1 or 0)
                     GL_BGR,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,    // Image data type
                     image.ptr());        // The actual image data itself
        return true;
    } else return false;
}*/

template<class T>
Eigen::Matrix<T,4,4> perspective(double fovy_x, double fovy_y, double zNear, double zFar) {
    assert(zFar > zNear);
    Eigen::Matrix<T,4,4> res = Eigen::Matrix<T,4,4>::Zero();
    res(0,0) = 1.0 / (tan(fovy_x / 2.0));
    res(1,1) = 1.0 / (tan(fovy_y / 2.0));
    res(2,2) = -(zFar + zNear) / (zFar - zNear);
    res(3,2) = -1.0;
    res(2,3) = -(2.0 * zFar * zNear) / (zFar - zNear);
    return res;
}


template<class T>
Eigen::Matrix<T,4,4> lookAt(Eigen::Matrix<T,3,1> const& eye,
                            Eigen::Matrix<T,3,1> const& center,
                            Eigen::Matrix<T,3,1> const & up) {
    const Eigen::Matrix<T,3,1> f = (center - eye).normalized();
    Eigen::Matrix<T,3,1> u = up.normalized();
    const Eigen::Matrix<T,3,1> s = f.cross(u).normalized();
    u = s.cross(f);
    
    Eigen::Matrix<T,4,4> res;
    res <<  s.x(), s.y(), s.z(), -s.dot(eye),
            u.x(), u.y(), u.z(), -u.dot(eye),
            -f.x(), -f.y(), -f.z(), f.dot(eye),
            0, 0, 0, 1;
    
    return res;
}

#endif  // _H_UTIL_

