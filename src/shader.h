/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#ifndef _H_SHADER_
#define _H_SHADER_

#include <string>

#include <nanogui/glutil.h>
#include <nanogui/opengl.h>

class Shader {
protected:
    bool initalized_{false};
public:
    nanogui::GLShader shader_{};
    void Init(const std::string& name, const std::string& vertex, const std::string fragment);
};

class Shader2D: public Shader {
public:
    void Init(const std::string& name);
};

class Shader3DColored: public Shader {
public:
    void Init(const std::string& name);
};

class Shader3DTextured: public Shader {
public:
    void Init(const std::string& name);
};

#endif
