#include "stdafx.h"
#include "Camera.h"
#include "ConsoleColors.h"

#include <algorithm>

Camera::Camera(glm::vec3 position) : position(position)
{
	this->position = { -2.5f, 2.3f, 2.3f };
	yaw = 320.f;
	pitch = -30.f;
	UpdateVectors();
} 

glm::mat4 Camera::ViewMatrix() const {
	return glm::lookAt(position, position+front, up);
}

void Camera::Move(CamMoveDir dir, float move_speed, float dt)
{
	position += getVelocity(dir, move_speed, dt);
}

glm::vec3 Camera::getVelocity(CamMoveDir dir, float move_speed, float dt)
{
	float speed = move_speed * dt;
	switch (dir)
	{
	case CamMoveDir::FORWARD:
		return horizontal_front * speed;
	case CamMoveDir::BACKWARD:
		return -horizontal_front * speed;
	case CamMoveDir::RIGHT:
		return right * speed;
	case CamMoveDir::LEFT:
		return -right * speed;
	case CamMoveDir::UP:
		return world_up * speed;
	case CamMoveDir::DOWN:
		return -world_up * speed;
	}
	return glm::vec3(0.f);
}

void Camera::UpdateLook(float dmx, float dmy)
{
	if (!first_mouse_input_recieved) {
		first_mouse_input_recieved = true;
		return;
	}
	dmx *= mouse_sensitivity;
	dmy *= -mouse_sensitivity;
	yaw += dmx;
	if (yaw >= 360.f) yaw = 0.f;
	else if (yaw <= 0.f) yaw = 360.f;
	pitch = std::min(89.f, std::max(-89.f, pitch + dmy));
	UpdateVectors();
}

void Camera::setPosition(glm::vec3 new_pos)
{
	position = new_pos;
}

void Camera::UpdateVectors()
{
	glm::vec3 _front;
	_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	_front.y = sin(glm::radians(pitch));
	_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(_front);
	horizontal_front = { _front.x, 0, _front.z };
	horizontal_front = glm::normalize(horizontal_front);
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}
