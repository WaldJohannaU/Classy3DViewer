/*******************************************************
 * Copyright (c) 2018, Johanna Wald
 * All rights reserved.
 *
 * This file is distributed under the GNU Lesser General Public License v3.0.
 * The complete license agreement can be obtained at:
 * http://www.gnu.org/licenses/lgpl-3.0.html
 ********************************************************/

#include "mouse_controls.h"
#include "util.h"

GUIMouseControls::GUIMouseControls(): eyeUp_(0,0,0) {
    view_.setIdentity();
}

void GUIMouseControls::Update() {
    if (button_left_) {
        UpdateLeft();
    }
    if (wheel_direction_ != 0) {
        wheel_distance_ += wheel_direction_;
        wheel_direction_ = 0;
    }
    // Direction: Spherical coordinates to Cartesian coordinates conversion
    Eigen::Vector3f direction(cos(vertical_angle_temp_) * sin(horizontal_angle_temp_),
                              sin(vertical_angle_temp_),
                              cos(vertical_angle_temp_) * cos(horizontal_angle_temp_));
    Eigen::Vector3f right = Eigen::Vector3f(sin(horizontal_angle_temp_ - M_PI/2.0f), 0, cos(horizontal_angle_temp_ - M_PI/2.0f));
    eyeUp_controls_ = right.cross(direction);
    if (button_right_) {
        UpdateRight(eyeUp_controls_, right);
    }
    // Right vector
    Eigen::Vector3f position_controls = pos_offset_control_temp_;
    look_at_controls_ = position_controls + 20 * direction;
    position_controls -= wheel_distance_ * 0.1f * direction;
    view_ = C3DV_camera::lookAt(position_controls, look_at_controls_, eyeUp_controls_);
}

void GUIMouseControls::Reset() {
    horizontal_angle_temp_ = horizontal_angle_;
    vertical_angle_temp_ = vertical_angle_;
    
    delta_pos_.setZero();
    pos_offset_control_temp_ = pos_offset_control_;
}

void GUIMouseControls::ButtonReleased() {
    button_pressed_ = false;
    end_pos_ = current_pos_;
    delta_pos_.setZero();
    horizontal_angle_ = horizontal_angle_temp_;
    vertical_angle_ = vertical_angle_temp_;
    pos_offset_control_ = pos_offset_control_temp_;
}

void GUIMouseControls::UpdateLeft() {
    horizontal_angle_temp_ = horizontal_angle_ + mouse_speed_ * delta_pos_.x();
    vertical_angle_temp_ = vertical_angle_ + mouse_speed_ * delta_pos_.y();
}

void GUIMouseControls::UpdateRight(const Eigen::Vector3f& eyeup_controls, const Eigen::Vector3f& right) {
    pos_offset_control_temp_ = pos_offset_control_ - delta_pos_.x() * 0.01f * right + delta_pos_.y() * 0.01f * eyeup_controls;
}

void GUIMouseControls::Pressed(const nanogui::Vector2i &position) {
    if (button_pressed_) {
        if (!initialized_)
            previous_pos_ = current_pos_;
        current_pos_[0] = static_cast<float>(position.x());
        current_pos_[1] = static_cast<float>(position.y());
        delta_pos_ = current_pos_ - previous_pos_;
        initialized_ = true;
    }
}

bool GUIMouseControls::ScrollCallbackEvent(double x, double y) {
    wheel_direction_ += y;
    return true;
}

bool GUIMouseControls::MouseButtonEvent(const nanogui::Vector2i &position, int button, bool down, int modifiers) {
    current_pos_[0] = static_cast<float>(position.x());
    current_pos_[1] = static_cast<float>(position.y());
    
    if (down) {
        button_pressed_ = true;
    } else {
        ButtonReleased();
    }
    if (button == GLFW_MOUSE_BUTTON_1) {
        button_left_ = button_pressed_;
    }
    else if (button == GLFW_MOUSE_BUTTON_2) {
        button_right_ = button_pressed_;
    }
    previous_pos_ = current_pos_;
    return true;
}
