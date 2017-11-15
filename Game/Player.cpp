#include "stdafx.h"
#include "Player.h"
#include <algorithm>

Player::Player() : move_speed(1.0f), gravity_acceleration_constant(1.f), friction_coefficient(0.3f)
{
	position = camera.getPosition();
	size = glm::vec3(0.5f, 2.f, 0.5f);
	relative_eye_position = glm::vec3(0.f, size.y / 2.f - 0.1f, 0.f);
}

void Player::UpdateMovement(GLFWwindow * window, float dt, std::vector<Collision::Box> boxes, std::vector<Collision::HalfBox> hboxes)
{
	// todo: rewrite this function to work with configurable controls

	std::vector<CamMoveDir> dirs;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::RIGHT);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::LEFT);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::UP);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		dirs.push_back(CamMoveDir::DOWN);

	glm::vec3 move_force(0.f);
	for (auto dir : dirs) { move_force += camera.getVelocity(dir, move_speed, 1); }
	move_force.y *= 1.9f;
	if (glm::vec2(move_force.x, move_force.z) != glm::vec2(0)) {
		glm::vec3 horizontal_move_force = 
			//glm::length(glm::vec3(move_force.x, 0, move_force.z)) *
			move_speed *
			glm::normalize(glm::vec3(move_force.x, 0, move_force.z));
		move_force.x = horizontal_move_force.x;
		move_force.z = horizontal_move_force.z;
	}

	glm::vec3 friction_force = glm::vec3(-1, 0, -1) * (velocity/dt) * friction_coefficient;
	glm::vec3 gravity_force =  glm::vec3(0, -1, 0) * gravity_acceleration_constant;
	glm::vec3 total_force = move_force + friction_force + gravity_force;

	velocity += total_force * dt;
	velocity.y = std::min(velocity.y, 0.3f);
	velocity.y = std::max(velocity.y, -0.4f);

	Collision::Box player_box;
	player_box.position = position + velocity;
	player_box.size = size;

	glm::vec3 mtv = glm::vec3(0);
	for (auto b : boxes) {
		auto res = Collision::Get(player_box, b);
		if (res.collision) mtv += res.mtv;
	}
	for (auto b : hboxes) {
		auto res = Collision::Get(player_box, b);
		if (res.collision) mtv += res.mtv;
	}

	velocity += mtv;


	if (gravity_acceleration_constant == 0.f) {
		velocity.y = move_force.y * 0.1f;
	}

	if (debug_fly_mode) {
		velocity = move_force * 0.2f;
		velocity.y /= 1.9f;
	}



	if (move_cam_only) { camera.setPosition(camera.getPosition() + move_force*10.f * dt); }
	else { position += velocity; }
	if (!fixed_cam) { camera.setPosition(position + relative_eye_position); }
}
