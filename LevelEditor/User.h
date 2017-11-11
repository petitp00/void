#pragma once
#include "../Engine/Camera.h"

class User
{
public:
	User();
	void UpdateMovement(GLFWwindow* window, float dt);

	Camera camera;
	float move_speed;
};

