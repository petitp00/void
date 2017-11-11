#include "stdafx.h"
#include "User.h"


User::User() : move_speed(3.0f)
{
}

void User::UpdateMovement(GLFWwindow * window, float dt)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.Move(CamMoveDir::FORWARD, move_speed, dt);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.Move(CamMoveDir::BACKWARD, move_speed, dt);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.Move(CamMoveDir::RIGHT, move_speed, dt);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.Move(CamMoveDir::LEFT, move_speed, dt);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.Move(CamMoveDir::UP, move_speed, dt);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.Move(CamMoveDir::DOWN, move_speed, dt);
	}
}
