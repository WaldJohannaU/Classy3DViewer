/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include "shader.h"

void Shader::Init(const std::string& name, const std::string& vertex, const std::string fragment) {
    if (!initalized_) {
        shader_.init(name, vertex, fragment);
        initalized_ = true;
    }
}

void Shader2D::Init(const std::string& name) {
    if (!initalized_) {
        const std::string& vertex = "#version 330\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec2 vertexUV;\n"
        "out vec2 UV;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "    UV = vertexUV;\n"
        "}";
        const std::string& fragment = "#version 330\n"
        "in vec2 UV;\n"
        "uniform sampler2D myTextureSampler;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(texture(myTextureSampler, UV).rgb, 1.0);\n"
        "}";
        
        shader_.init(name, vertex, fragment);
        
        unsigned int indices[] = {0, 1, 2, 2, 3, 0};
        float vertices[] = {-1, -1, 1, 1, -1, 1, 1,  1, 1, -1,  1, 1};
        float uvs[] = {0, 1, 1, 1, 1, 0, 0, 0};
        float uvs_flipped[] = {1, 0, 0, 0, 0, 1, 1, 1};
        
        Eigen::Map<nanogui::MatrixXf> v_vertices(vertices,3,4);
        Eigen::Map<nanogui::MatrixXf> v_uvs(uvs,2,4);
        Eigen::Map<nanogui::MatrixXf> v_uvs_flipped(uvs_flipped,2,4);
        Eigen::Map<nanogui::MatrixXu> v_indices(indices,3,2);

        shader_.bind();
        shader_.uploadIndices(v_indices);
        shader_.uploadAttrib("position", v_vertices);
        shader_.uploadAttrib("vertexUV", v_uvs);
        initalized_ = true;
    }
}

void Shader3DColored::Init(const std::string& name) {
    if (!initalized_) {
        const std::string vertex{"#version 330\n"
        "uniform mat4 model_view_projection;\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 color;\n"
        "out vec3 colorV;\n"
        "void main() {\n"
        "    gl_Position = model_view_projection * vec4(position, 1.0);\n"
        "    colorV = color;\n"
            "}"};
        
        const std::string fragment{"#version 330\n"
        "in vec3 colorV;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(colorV, 1.0);\n"
        "}"};
        
        shader_.init(name, vertex, fragment);
        initalized_ = true;
    }
}

void Shader3DTextured::Init(const std::string& name) {
    if (!initalized_) {
        const std::string& vertex{"#version 330\n"
            "uniform mat4 model_view_projection;\n"
            "layout(location = 0) in vec3 position;\n"
            "layout(location = 1) in vec2 vertexUV;\n"
            "layout(location = 2) in vec3 color;\n"
            "out vec3 colorV;\n"
            "out vec2 UV;\n"
            "void main() {\n"
            "    gl_Position = model_view_projection * vec4(position, 1.0);\n"
            "    colorV = color;\n"
            "    UV = vec2(vertexUV.x, 1 - vertexUV.y);\n"
            "}"};
        
        const std::string& fragment{"#version 330\n"
            "in vec2 UV;\n"
            "in vec3 colorV;\n"
            "uniform sampler2D myTextureSampler;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "    color = vec4(texture(myTextureSampler, UV).rgb, 1.0);\n"
            "}"};
        
        shader_.init(name, vertex, fragment);
        initalized_ = true;
    }
}
