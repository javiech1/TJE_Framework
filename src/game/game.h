/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "framework/utils.h"
#include "game/stages/stage.h"
#include "framework/extra/bass.h"  // For HCHANNEL type

struct ThirdPersonCameraState 
{
	float yaw, pitch;
	float distance;
	float height_offset;
	Vector3 eye;
	Vector3 focus;
};

class Game
{
	public:
		static Game* instance;
		
		Stage* current_stage = nullptr;

		ThirdPersonCameraState camera_state;

		SDL_Window* window;
		int window_width;
		int window_height;

		long frame;
		float time;
		float elapsed_time;
		int fps;
		bool must_exit;

		Camera* camera;
		bool mouse_locked;

		Game( int window_width, int window_height, SDL_Window* window );

		void render( void );
		void update( double dt );
		void setMouseLocked(bool must_lock);

		void onKeyDown( SDL_KeyboardEvent event );
		void onKeyUp(SDL_KeyboardEvent event);
		void onMouseButtonDown( SDL_MouseButtonEvent event );
		void onMouseButtonUp(SDL_MouseButtonEvent event);
		void onMouseWheel(SDL_MouseWheelEvent event);
		void onGamepadButtonDown(SDL_JoyButtonEvent event);
		void onGamepadButtonUp(SDL_JoyButtonEvent event);
		void onResize(int width, int height);
		void onMouseMove(SDL_MouseMotionEvent event);
		void setStage(Stage* new_stage);

	private:
		void updateThirdPersonCamera(EntityPlayer* player, float dt);
		HCHANNEL global_music_channel = 0;
};
