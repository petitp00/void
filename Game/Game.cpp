#include "stdafx.h"
#include "Game.h"
#include "CubeRenderer.h"
#include "PointRenderer.h"
#include "Collisions.h"
#include "LineRenderer.h"

#include <filesystem>
using namespace std::experimental::filesystem;

int main(int argc, char *argv[])
{
	print_blue("wew");
		
	RESOURCES_FOLDER = "../../Resources/";

	// detects if game has been launched from vs (cwd is C:/void/Game/)
	path cur_path = current_path();
	auto folder = cur_path.end();
	if (folder != cur_path.begin()) {
		--folder;
		std::cout << folder->string() << '\n';
		if (folder->string() == "Game") {
			RESOURCES_FOLDER = "../Resources/";
		}
	}

	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Game game;
	game.Start();

	glfwTerminate();
    return 0;
}

Game::Game() : window_size(1280, 720)
{
	if (enable_multisampling)
		glfwWindowHint(GLFW_SAMPLES, multisampling_level);
	
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(window_size.x, window_size.y, "Game", 0, 0);
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
	glEnable(GL_MULTISAMPLE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClearColor(0.0f, 0.f, 0.f, 1.0f);
	//glClearColor(1.0f, 1.f, 1.f, 1.0f);

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
	CubeRenderer::Init(window_size);

	InitPostProcFbo(window_size);
	post_proc_shader.Init("post_proc");
	glGenVertexArrays(1, &vao_quad);
	float quad_data[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	uint vbo_quad;
	glGenBuffers(1, &vbo_quad);
	glBindVertexArray(vao_quad);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Game::Start()
{
	double last_time = glfwGetTime();
	float fps = 0.f;
	double frame_time = 0.f;
	double accumulator = 0.f;

	float fps_average = 0.f;
	std::vector<float> last_fps_vals;
	float last_fps_refresh = (float)glfwGetTime();
	float fps_refresh_time = 0.25f; // in seconds

	Cube c1;
	c1.position = glm::vec3(0.5f);
	c1.position.y = 0.f;
	c1.size = glm::vec3(1.f);
	//c1.position.y = 4.f;
	//c1.position = glm::vec3(0.f);
	Cube c2 = c1;
	c2.position = glm::vec3(2.75f, 1.f, 0.f);
	c2.size = glm::vec3(0.5f, 3.f, 8.f);
	Cube c3;
	c3.size = glm::vec3(1006.f, 0.5f, 28.f);
	c3.position = glm::vec3(0.f, -0.75f, 0.f);
	std::vector<Cube> cubes = { c1,  c2, c3};


	std::vector<HalfCube> hcubes;
	for (int i = 1; i != 30; ++i) {
		HalfCube hc;
		hc.size = glm::vec3(2, 0.25f * i, 0.95f);
		hc.position = glm::vec3(-10.f, c3.position.y + c3.size.y*0.5f + hc.size.y / 2.f, -5.f + i);
		hc.rotation = glm::vec2(1, 0);
		hcubes.push_back(hc);
		hc.rotation = glm::vec2(-1, 0);
		hc.position.x += hc.size.x;
		hc.size.y = 0.25f * (30 - i);
		hcubes.push_back(hc);
	}

	std::vector<Collision::Box> boxes;
	boxes.push_back(Collision::Box(c1));
	boxes.push_back(Collision::Box(c2));
	boxes.push_back(Collision::Box(c3));

	std::vector<Collision::HalfBox> hboxes;
	for (auto hc : hcubes) {
		Collision::HalfBox hbox(hc);
		hboxes.push_back(hbox);
	}

	PointRenderer::Init();
	Point po;
	po.color = { 1.0f, 0.f, 1.0f, 1.f };
	po.display_position_text = true;
	Point px, py, pz;
	px.display_position_text = true;
	py.display_position_text = true;
	pz.display_position_text = true;
	px.position = { 1.f, 0.f, 0.f };
	py.position = { 0.f, 1.f, 0.f };
	pz.position = { 0.f, 0.f, 1.f };
	px.color = { 1.f, 0.f, 0.f, 1.f };
	py.color = { 0.f, 1.f, 0.f, 1.f };
	pz.color = { 0.f, 0.f, 1.f, 1.f };
	Point pc2 = po;
	pc2.position = { 4, 1, 0 };
	std::vector<Point> points = { po, px, py, pz };

	Cube player_cube;
	player_cube.size = player.size;
	player_cube.position = player.position;
	player_cube.mat.diffuse = glm::vec4(0.2f, 0.9f, 0.0f, 1.0f);

	LineRenderer::Init();
	Line slope, perp;
	slope.color = glm::vec4(1.f, 0.f, 1.f, 1.f);
	perp.color = glm::vec4(1.f, 1.f, 0.f, 1.f);
	std::vector<Line> lines = { slope, perp };

	while (!glfwWindowShouldClose(window))
	{
		// time calculations and fps update
		{
			double current_time = glfwGetTime();
			frame_time = current_time - last_time;
			last_time = current_time;
			accumulator += frame_time;

			fps = 1 / (float)frame_time;
			last_fps_vals.push_back(fps);
			
			if (current_time - last_fps_refresh >= fps_refresh_time) {
				last_fps_refresh = (float)current_time;
				float sum = 0.f;
				for (auto n : last_fps_vals) sum += n;
				fps_average = sum / last_fps_vals.size();
				last_fps_vals.clear();
			}
		}

		glfwPollEvents();

		// UPDATE

		while (accumulator >= dt)
		{
			accumulator -= dt;

			player.UpdateMovement(window, dt, boxes, hboxes);

			player_cube.position = player.position;

			Collision::Box player_box, other_box;
			player_box.position = player_cube.position;
			player_box.size = player_cube.size;
			other_box.position = c1.position;
			other_box.size = c1.size;

			auto coll_res = Collision::Get(player_box, other_box);
			if (coll_res.collision) {
				player_cube.mat.diffuse = glm::vec4(0.9, 0.2, 0.0, 1.0);
			}
			else {
				player_cube.mat.diffuse = glm::vec4(0.2f, 0.9f, 0.0f, 1.0f);
			}
		}

		// RENDER
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 projection = glm::perspective(glm::radians(player.camera.getFov()),
			(float)window_size.x / (float)window_size.y, 0.1f, 100.f);
		RenderInfo cube_render_info;
		cube_render_info.proj = projection;
		cube_render_info.view = player.camera.ViewMatrix();
		cube_render_info.view_pos = player.camera.getPosition();
		cube_render_info.light.position = player.camera.getPosition();
		cube_render_info.light.directional = false;
		cube_render_info.light.ambient = glm::vec3(0.2f);
		cube_render_info.framebuffer_size = window_size;

		if (player.fixed_cam) {
			CubeRenderer::AddCubesToDraw({ player_cube }, {});
		}

		CubeRenderer::AddCubesToDraw(cubes, hcubes);
		CubeRenderer::RenderFrame(cube_render_info, post_proc_fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		post_proc_shader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, post_proc_col_buf);
		post_proc_shader.setInt("screenTex", 0);
		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		PointRenderInfo point_render_info;
		point_render_info.projection = projection;
		point_render_info.view = player.camera.ViewMatrix();
		point_render_info.text_renderer = &text_renderer;
		point_render_info.view_pos = player.camera.getPosition();

		LineRenderInfo line_render_info;
		line_render_info.projection = projection;
		line_render_info.view = player.camera.ViewMatrix();

		glDisable(GL_DEPTH_TEST);
			PointRenderer::Draw(points, point_render_info);
			LineRenderer::Draw(lines, line_render_info);

			if (show_debug_gui) {
				float text_y = 5.f;
				glm::vec3 color(0.5f, 0.8f, 0.2f);
				std::stringstream ss;
				
				text_renderer.RenderText("fps: " + std::to_string(int(fps_average)),
					{ 5.f, text_y }, 0.5f, color);
				text_y += 15.f;

				text_renderer.RenderText("dt: " + std::to_string(dt),
					{ 5.f, text_y }, 0.5f, color);
				text_y += 15.f;

				ss << player.velocity;
				text_renderer.RenderText("velocity: " + ss.str(),
					{ 5.f, text_y }, 0.5f, color);
				text_y += 15.f;

			}
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);

		assert(glGetError() == GL_NO_ERROR);
	}
}

void Game::InitPostProcFbo(glm::ivec2 framebuffer_size)
{
	if (post_proc_fbo != 0) {
		glDeleteFramebuffers(1, &post_proc_fbo);
		glDeleteTextures(1, &post_proc_col_buf);
	}
	glGenFramebuffers(1, &post_proc_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, post_proc_fbo);
	glGenTextures(1, &post_proc_col_buf);
	glBindTexture(GL_TEXTURE_2D, post_proc_col_buf);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebuffer_size.x, framebuffer_size.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, post_proc_col_buf, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		print_red("Framebuffer not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Event Callback Functions
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	game->window_size = { width, height };
	glViewport(0, 0, width, height);
	game->text_renderer.ChangeFramebufferSize(game->window_size);
	CubeRenderer::ChangeFramebufferSize(game->window_size);
	game->InitPostProcFbo(game->window_size);
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, true);
		}
		else if (key == GLFW_KEY_TAB) {
			game->mouse_captured = !game->mouse_captured;
			if (game->mouse_captured)
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
			auto current_pos = game->player.camera.getPosition();
			glm::vec3 new_pos = current_pos;
			new_pos.y = 2.f;
			game->player.camera.setPosition(new_pos);
		}
		else if (key == GLFW_KEY_SPACE) {
			game->player.fixed_cam = !game->player.fixed_cam;
		}
		else if (key == GLFW_KEY_LEFT_ALT) {
			game->player.move_cam_only = true;
		}
		else if (key == GLFW_KEY_F1) {
			game->enable_vsync = !game->enable_vsync;
			if (game->enable_vsync) {
				glfwSwapInterval(1);
			}
			else {
				glfwSwapInterval(0);
			}
		}
		else if (key == GLFW_KEY_F2) {
			game->player.debug_fly_mode = !game->player.debug_fly_mode;
		}
		else if (key == GLFW_KEY_F3) {
			game->player.gravity_acceleration_constant = 0.f;
		}
		else if (key == GLFW_KEY_F5) {
			game->post_proc_shader.Init("post_proc");
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT_ALT) {
			game->player.move_cam_only = false;
		}
	}
}

void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (game->mouse_captured) {
		float dmx = (float)xpos - game->mouse_pos.x;
		float dmy = (float)ypos - game->mouse_pos.y;
		game->player.camera.UpdateLook(dmx, dmy);
	}
	game->mouse_pos = { float(xpos), float(ypos) };
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		game->player.move_cam_only = true;
	}
	else if (action == GLFW_RELEASE) {
		game->player.move_cam_only = false;
	}
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
}
