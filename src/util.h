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

namespace C3DV_camera {

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

};

#endif  // _H_UTIL_

