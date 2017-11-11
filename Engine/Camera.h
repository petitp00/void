#pragma once

enum class CamMoveDir {
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT,
	UP,
	DOWN
};

class Camera
{
public:
	Camera(glm::vec3 position = { 0.f, 0.f, 0.f });
	glm::mat4 ViewMatrix() const;
	void Move(CamMoveDir dir, float move_speed, float dt);
	glm::vec3 getVelocity(CamMoveDir dir, float move_speed, float dt);
	void UpdateLook(float dmx, float dmy);

	glm::vec3 getPosition() const { return position; }
	float getFov() const { return fov; }

	void setPosition(glm::vec3 new_pos);

private:
	void UpdateVectors();

	glm::vec3 position;
	glm::vec3 front, horizontal_front;
	glm::vec3 up, right;
	glm::vec3 world_up = { 0.f, 1.f, 0.f };

	float yaw = -90.f;
	float pitch = 90.f;

	float mouse_sensitivity = 0.1f;
	float fov = 55.f;

	bool first_mouse_input_recieved = false;
};

