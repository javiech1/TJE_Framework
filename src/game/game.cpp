#include "game.h"
#include "framework/utils.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "game/stages/play_stage.h"
#include "game/stages/menu_stage.h"
#include "framework/audio.h"

#include <cmath>
#include <algorithm>

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = true;

	camera_state.distance = 12.0f;
	camera_state.yaw = float(M_PI);
	camera_state.pitch = -0.4f;
	camera_state.height_offset = 2.0f;
	camera_state.eye = Vector3(0.0f, 10.0f, -20.0f);
	camera_state.focus = Vector3(0.0f, 5.0f, 0.0f);

	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	if (Audio::Init()) {
		global_music_channel = Audio::Play("data/audio/stellar_drift.mp3", 0.4f, BASS_SAMPLE_LOOP);
	}

	camera = new Camera();
	camera->lookAt(Vector3(0.f,10.f, -15.f),Vector3(0.f,5.f,0.f), Vector3(0.f,1.f,0.f));
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f);

	setStage(new MenuStage());
}

void Game::render(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->enable();

	if(current_stage){
		current_stage->render(camera);
	}

	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	if(current_stage){
		current_stage->update(seconds_elapsed);
	}

	const float mouse_sensitivity = 0.004f;
	if (mouse_locked) {
		camera_state.yaw   -= Input::mouse_delta.x * mouse_sensitivity;
		camera_state.pitch -= Input::mouse_delta.y * mouse_sensitivity;
	}

	const float rotate_speed = float(seconds_elapsed) * 1.5f;
	if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera_state.yaw += rotate_speed;
	if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera_state.yaw -= rotate_speed;
	if (Input::isKeyPressed(SDL_SCANCODE_UP)) camera_state.pitch += rotate_speed;
	if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera_state.pitch -= rotate_speed;

	camera_state.pitch = std::max(-1.2f, std::min(0.4f, camera_state.pitch));

	if (Input::isKeyPressed(SDL_SCANCODE_PAGEUP))
		camera_state.distance = std::max(3.0f, camera_state.distance - float(seconds_elapsed) * 5.0f);
	if (Input::isKeyPressed(SDL_SCANCODE_PAGEDOWN))
		camera_state.distance = std::min(20.0f, camera_state.distance + float(seconds_elapsed) * 5.0f);

	Vector3 player_pos = Vector3();
	float player_scale = 1.0f;
	EntityPlayer* player = nullptr;
	if (current_stage) {
		player = current_stage->getPlayer();
		if (player) {
			player_pos = player->getPosition();
			player_scale = player->getScale();
		}
	}
	player_scale = std::max(0.001f, player_scale);
	camera_state.distance = std::max(camera_state.distance, player_scale * 2.0f);

	updateThirdPersonCamera(player, float(seconds_elapsed));
}

void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_F1: Shader::ReloadAll(); break;
	}

	if(current_stage){
		current_stage->onKeyDown(event);
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	if(current_stage){
		current_stage->onKeyUp(event);
	}
}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) {
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
	}
}

void Game::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	// Implementar si se necesita
}

void Game::onMouseWheel( SDL_MouseWheelEvent event )
{
	// Implementar si se necesita
}

void Game::onGamepadButtonDown( SDL_JoyButtonEvent event )
{
	// Implementar si se necesita
}

void Game::onGamepadButtonUp( SDL_JoyButtonEvent event )
{
	// Implementar si se necesita
}

void Game::onResize(int width, int height)
{
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

void Game::setMouseLocked(bool must_lock)
{
	SDL_ShowCursor(!must_lock);

	SDL_SetRelativeMouseMode((SDL_bool)must_lock);

	mouse_locked = must_lock;
}

void Game::onMouseMove(SDL_MouseMotionEvent event)
{
	if(current_stage){
		current_stage->onMouseMove(event);
	}
}

void Game::setStage(Stage* new_stage)
{
	if(current_stage)
		delete current_stage;
	current_stage = new_stage;
}

void Game::updateThirdPersonCamera(EntityPlayer* player, float dt)
{
	if(!player) return;

	Vector3 player_pos = player ? player->getPosition() : Vector3();
	float player_scale = player ? player->getScale() : 1.0f;
	Vector3 target = player_pos + Vector3(0.0f, player_scale * 0.5f, 0.0f);

	Vector3 offset;
	offset.x = std::cos(camera_state.pitch) * std::sin(camera_state.yaw);
	offset.y = std::sin(camera_state.pitch);
	offset.z = std::cos(camera_state.pitch) * std::cos(camera_state.yaw);

	if(offset.length() < 0.001f)
		offset = Vector3(0.0f, 0.0f, 1.0f);
	offset.normalize();

	Vector3 desired_eye = target - offset * camera_state.distance + Vector3(0.0f, camera_state.height_offset + player_scale * 0.5f, 0.0f);

	const float smooth = std::min(1.0f, dt * 5.0f);
	camera_state.eye = lerp(camera_state.eye, desired_eye, smooth);
	camera_state.focus = lerp(camera_state.focus, target, smooth);

	camera->lookAt(camera_state.eye, camera_state.focus, Vector3(0.0f, 1.0f, 0.0f));
}
