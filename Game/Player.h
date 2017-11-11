#pragma once

#include "../Engine/Camera.h"
#include "../Engine/Collisions.h"

class Player
{
public:
	Player();
	void UpdateMovement(GLFWwindow* window, float dt, std::vector<Collision::Box> boxes, std::vector<Collision::HalfBox> hboxes, glm::vec3& start1, glm::vec3& end1, glm::vec3& start2, glm::vec3& end2);
	
	Camera camera;

	glm::vec3 position;
	glm::vec3 size;

	glm::vec3 relative_eye_position;

	glm::vec3 velocity; // for dt = 1

	float gravity_acceleration_constant;
	float friction_coefficient;
	float move_speed;

	bool debug_fly_mode = false;
	bool fixed_cam = false;
	bool move_cam_only = false;

};

