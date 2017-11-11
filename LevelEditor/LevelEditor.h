#pragma once

#include "../Engine/TextRenderer.h"
#include "User.h"
#include "Crosshair.h"

class LevelEditor
{
public:
	LevelEditor();
	void Start();

private:
	// Event callback functions
	/// they are friend functions because glfw requires them to be functions
	/// but we still want access to everything in Game
	friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	friend void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	// Window
	GLFWwindow* window;
	glm::ivec2 window_size;

	// Stuff (to be renamed)
	TextRenderer text_renderer;
	User user;
	Crosshair crosshair;

	// Settings
	bool show_debug_gui				= true;
	const bool enable_multisampling	= true;
	const byte multisampling_level	= 8;
	const bool enable_vsync			= false;
	const bool start_maximized		= true;

	// global state
	/// mouse
	bool mouse_captured = true;
	glm::vec2 mouse_pos = { 0,0 };

	float dt = 0.f;
	
};

// Event callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
