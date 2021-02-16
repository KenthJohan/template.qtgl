#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_sdlglew.h"

//#include "api.h"

#include <flecs.h>
#include <posix_set_os_api.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <GL/glew.h>
#include <stdio.h>


#include "components.h"
#include "systems.h"


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Demo3"








static void addents (ecs_world_t * world)
{

	ECS_ENTITY (world, mytexture1, component_tbo);
	ECS_ENTITY (world, mytexture2, component_tbo);
	ECS_ENTITY (world, mytexture3, component_tbo);
	ecs_set (world, mytexture1, component_texture, {.unit = 0, .width = 100, .height = 100, .depth = 4});
	ecs_set (world, mytexture2, component_texture, {.unit = 0, .width = 50, .height = 50, .depth = 4});
	ecs_set (world, mytexture3, component_texture, {.unit = 0, .width = 200, .height = 200, .depth = 4});

	ECS_ENTITY (world, img, component_mesh, component_vao);
	ecs_set (world, img, component_count, {6});
	ecs_set (world, img, component_rectangle, {1.0f, 1.0f});

	ecs_entity_t const * e2 = ecs_bulk_new (world, 0, 4);
	ecs_add_entity (world, e2[0], ECS_INSTANCEOF | mytexture1);
	ecs_add_entity (world, e2[1], ECS_INSTANCEOF | mytexture1);
	ecs_add_entity (world, e2[2], ECS_INSTANCEOF | mytexture2);
	ecs_add_entity (world, e2[3], ECS_INSTANCEOF | mytexture3);

	ecs_add_entity (world, e2[0], ECS_INSTANCEOF | img);
	ecs_add_entity (world, e2[1], ECS_INSTANCEOF | img);
	ecs_add_entity (world, e2[2], ECS_INSTANCEOF | img);
	ecs_add_entity (world, e2[3], ECS_INSTANCEOF | img);
	for (int i = 0; i < 4; ++i)
	{
		ecs_set (world, e2[i], component_scale, {(float)(i+1)/10.0f, (float)(i+1)/10.0f, 1.0f, 1.0f});
		ecs_set (world, e2[i], component_quaternion, {1.0f, 0.0f, 0.0f, 0.0f});
		ecs_set (world, e2[i], component_position, {0.0f, 0.0f, (float)i/2.0f, 0.0f});
	}
}




int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
	ASSERT (argc);
	ASSERT (argv);


	uint32_t main_flags = CSC_SDLGLEW_RUNNING;

	SDL_Window * window;
	SDL_GLContext context;
	csc_sdlglew_create_window (&window, &context, WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);

	glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable (GL_BLEND);
	glEnable (GL_DEPTH_TEST);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	ecs_world_t * world = ecs_init();
	systems_init (world);
	addents (world);
	//ecs_entity_t e3 = e2[0];
	const uint8_t * keyboard = SDL_GetKeyboardState (NULL);
	ecs_singleton_set (world, component_controller, {keyboard});




	while (main_flags & CSC_SDLGLEW_RUNNING)
	{
		SDL_Event event;
		while (SDL_PollEvent (&event))
		{
			csc_sdlglew_event_loop (window, &event, &main_flags, &global_gcam);
		}

		{
			//Control graphics camera
			csc_sdl_motion_wasd (keyboard, global_gcam.d);
			csc_sdl_motion_pyr (keyboard, global_gcam.pyrd);
			v3f32_mul (global_gcam.d, global_gcam.d, 0.01f);
			v3f32_mul (global_gcam.pyrd, global_gcam.pyrd, 0.01f);
			csc_gcam_update (&global_gcam);
		}

		glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		if (keyboard[SDL_SCANCODE_1])
		{
			//qf32_rotate2_xyza (*ecs_get_mut (global_world, e3, component_quaternion, NULL), keyboard[SDL_SCANCODE_1], keyboard[SDL_SCANCODE_2], keyboard[SDL_SCANCODE_3], 0.01f);
		}

		//if (keyboard[SDL_SCANCODE_2])
		{
			//qf32_rotate2_xyza (*ecs_get_mut (world, e3, component_quaternion, NULL), 0.0f, 1.0f, 0.0f, 0.01f);
		}

		//if (keyboard[SDL_SCANCODE_3])
		{
			//qf32_rotate2_xyza (*ecs_get_mut (world, e3, component_quaternion, NULL), 0.0f, 0.0f, 1.0f, 0.01f);
		}

		/*
		glUniform1i (uniform_texture1, 0);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) gcam.mvp);
		glDrawArrays (GL_TRIANGLES, 0, img.cap * CSC_GLIMAGE_VERTS_COUNT);
		*/


		ecs_progress (world, 0);
		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_GL_DeleteContext (context);
	SDL_DestroyWindow (window);
	SDL_Quit ();
	ecs_fini (world);
	return 0;
}
