#include "stdafx.h"
#include "LevelEditor.h"

#include "../Engine/LineRenderer.h"
#include "../Engine/PointRenderer.h"

#include "Grid.h"
#include "Crosshair.h"

int main()
{
	print_green("level wew editor");

	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	LevelEditor editor;
	editor.Start();

	glfwTerminate();
    return 0;
}

LevelEditor::LevelEditor() : window_size(1280, 720)
{
	if (enable_multisampling)
		glfwWindowHint(GLFW_SAMPLES, multisampling_level);

	window = glfwCreateWindow(window_size.x, window_size.y, "Editor", 0, 0);
	if (window == nullptr) {
		print_red("ERROR: Failed to create window.");
		return;
	}

	glfwMakeContextCurrent(window);

	if (!enable_vsync)
		glfwSwapInterval(0); // must be after glfwMakeContextCurrent

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		print_red("ERROR: gladLoadGLLoader failed.");
		return;
	}

	glViewport(0, 0, window_size.x, window_size.y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// set event callbacks
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	if (mouse_captured)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	text_renderer.Init(window_size, "tahoma.ttf", 24);
	
	// must be after setting events callbacks and initializing text_renderer
	if (start_maximized)
		glfwMaximizeWindow(window);

	LineRenderer::Init();
	PointRenderer::Init();
}

void LevelEditor::Start()
{
	// Time stuff
	float last_time = (float)glfwGetTime();
	float fps = 0.f;
	float fps_average = 0.f;
	std::vector<float> last_fps_vals;
	float last_fps_refresh = (float)glfwGetTime();
	float fps_refresh_time = 0.25f; // in seconds

	Line lx, ly, lz;
	lx.end = { 1,0,0 };
	ly.end = { 0,1,0 };
	lz.end = { 0,0,1 };
	lx.color = { 1,0,0,1 };
	ly.color = { 0,1,0,1 };
	lz.color = { 0,0,1,1 };
	lx.thickness = 0.2f;
	ly.thickness = 0.2f;
	lz.thickness = 0.2f;

	Point po, px, py, pz;
	po.display_position_text = true;
	px.position = { 1.f, 0.f, 0.f };
	py.position = { 0.f, 1.f, 0.f };
	pz.position = { 0.f, 0.f, 1.f };
	px.color = { 1.f, 0.f, 0.f, 1.f };
	py.color = { 0.f, 1.f, 0.f, 1.f };
	pz.color = { 0.f, 0.f, 1.f, 1.f };
	po.color = glm::vec4(1, 0, 1, 1);

	Grid grid(100, 0.5f);
	Grid grid2(100 * 2, 0.25f, { 0.2,0.2,0.2,1 }, 0.05f);

	crosshair.Init(window_size);

	while (!glfwWindowShouldClose(window))
	{
		// time calculations and fps update
		{
			float current_time = (float)glfwGetTime();
			float dt_real = current_time - last_time;
			last_time = current_time;
			dt = dt_real;
			fps = 1 / dt_real;
			last_fps_vals.push_back(fps);

			if (current_time - last_fps_refresh >= fps_refresh_time) {
				last_fps_refresh = current_time;
				float sum = 0.f;
				for (auto n : last_fps_vals) sum += n;
				fps_average = sum / last_fps_vals.size();
				last_fps_vals.clear();
			}
		}

		glfwPollEvents();

		// UPDATE
		glm::mat4 projection = glm::perspective(glm::radians(user.camera.getFov()), (float)window_size.x / (float)window_size.y, 0.1f, 100.f);
		user.UpdateMovement(window, dt);
		
		LineRenderInfo lri;
		lri.projection = projection;
		lri.view = user.camera.ViewMatrix();

		PointRenderInfo pri;
		pri.projection = projection;
		pri.view = user.camera.ViewMatrix();
		pri.text_renderer = &text_renderer;
		pri.view_pos = user.camera.getPosition();

		// RENDER
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		grid.Draw(lri);
		grid2.Draw(lri);

		LineRenderer::Draw({ lx, ly, lz }, lri);
		//LineRenderer::Draw(grid, lri);
		PointRenderer::Draw({ po, px, py, pz }, pri);

		crosshair.Draw();

		if (show_debug_gui) {
			float text_y = 5.f;
			glm::vec3 color(0.5f, 0.8f, 0.2f);

			text_renderer.RenderText("fps: " + std::to_string(int(fps_average)),
			{ 5.f, text_y }, 0.5f, color);
			text_y += 15.f;
		}

		glfwSwapBuffers(window);
	}
}

// Event Callback Functions
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	LevelEditor* editor = static_cast<LevelEditor*>(glfwGetWindowUserPointer(window));
	editor->window_size = { width, height };
	glViewport(0, 0, width, height);
	editor->text_renderer.ChangeFrameBufferSize(editor->window_size);
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	LevelEditor* editor = static_cast<LevelEditor*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, true);
		}
		else if (key == GLFW_KEY_TAB) {
			editor->mouse_captured = !editor->mouse_captured;
			if (editor->mouse_captured)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else if (key == GLFW_KEY_F11) {
			if (!glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
				glfwMaximizeWindow(window);
			else
				glfwRestoreWindow(window);
		}
		else if (key == GLFW_KEY_RIGHT_SHIFT) {
			auto current_pos = editor->user.camera.getPosition();
			glm::vec3 new_pos = current_pos;
			new_pos.y = 2.f;
			editor->user.camera.setPosition(new_pos);
		}
	}
}

void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	LevelEditor* editor = static_cast<LevelEditor*>(glfwGetWindowUserPointer(window));
	if (editor->mouse_captured) {
		float dmx = (float)xpos - editor->mouse_pos.x;
		float dmy = (float)ypos - editor->mouse_pos.y;
		editor->user.camera.UpdateLook(dmx, dmy);
	}
	editor->mouse_pos = { float(xpos), float(ypos) };
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	LevelEditor* editor = static_cast<LevelEditor*>(glfwGetWindowUserPointer(window));
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	LevelEditor* editor = static_cast<LevelEditor*>(glfwGetWindowUserPointer(window));
}
