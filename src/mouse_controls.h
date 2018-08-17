/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#ifndef _H_MOUSE_CONTROLS_
#define _H_MOUSE_CONTROLS_

#include <Eigen/Dense>
#include <nanogui/common.h>

class GUIMouseControls {
private:
    const float mouse_speed_ = 0.002f;
    
    float horizontal_angle_ = -M_PI;
    float vertical_angle_ = M_PI/3;
    
    float horizontal_angle_temp_;
    float vertical_angle_temp_;
    
    Eigen::Vector2f delta_pos_;
    Eigen::Vector2f current_pos_;
    Eigen::Vector2f previous_pos_;
    Eigen::Vector2f init_pos_;
    Eigen::Vector2f end_pos_;
   
    Eigen::Vector3f eyeUp_;
    Eigen::Vector3f eyeUp_controls_; // Head is up (set to 0,-1,0 to look upside-down)
    Eigen::Vector3f look_at_controls_;
    Eigen::Vector3f look_at_camera_;
    
    // mouse controls:
    int wheel_direction_{0};
    bool button_left_{false};
    bool button_right_{false};
    bool button_pressed_{false};
    float wheel_distance_{400.0f};
    Eigen::Vector3f pos_offset_control_temp_{0,0,0};
    Eigen::Vector3f pos_offset_control_{0,0,0};
    
    bool initialized_{false};
public:
    Eigen::Matrix4f view_;

    GUIMouseControls();
    void Update();
    void Reset();
    void ButtonReleased();
    void UpdateLeft();
    void UpdateRight(const Eigen::Vector3f& eyeup_controls, const Eigen::Vector3f& right);
    void Pressed(const nanogui::Vector2i &position);
    bool ScrollCallbackEvent(double x, double y);
    bool MouseButtonEvent(const nanogui::Vector2i &position, int button, bool down, int modifiers);
};

#endif
