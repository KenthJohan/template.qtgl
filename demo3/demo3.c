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


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Demo3"




#define COMP_POS 0
#define COMP_VEL 1
#define COMP_MASS 2


static ecs_world_t * world;


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






	world = ecs_init();
	components_init (world);

	ECS_TYPE (world, type_points, component_position, tag_glpoints);
	ecs_entity_t const * e1 = ecs_bulk_new (world, type_points, 100);
	for (int i = 0; i < 100; ++i)
	{
		ecs_set (world, e1[i], component_position, {(float)i/10.0f, 0.0f, 0.0f, 10.0f});
	}

	ECS_TYPE (world, type_imgs, component_position, component_scale, component_quaternion, tag_glimgs);
	ecs_entity_t const * e2 = ecs_bulk_new (world, type_imgs, 1);
	for (int i = 0; i < 1; ++i)
	{
		ecs_set (world, e2[i], component_position, {0.0f, 0.0f, (float)i/2.0f, 0.0f});
		ecs_set (world, e2[i], component_scale, {1.0f, (float)i/2.0f + 1.0f, 1.0f, 1.0f});
		ecs_set (world, e2[i], component_quaternion, {1.0f, 0.0f, 0.0f, 0.0f});
	}
	ecs_entity_t e3 = e2[0];


	const Uint8 * keyboard = SDL_GetKeyboardState (NULL);
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
			vsf32_mul (3, global_gcam.d, global_gcam.d, 0.01f);
			vsf32_mul (3, global_gcam.pyrd, global_gcam.pyrd, 0.01f);
			csc_gcam_update (&global_gcam);
		}

		glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		if (keyboard[SDL_SCANCODE_1])
		{
			component_quaternion * q = ecs_get_mut (world, e3, component_quaternion, NULL);
			component_quaternion q0;
			qf32_xyza ((void*)&q0, 1.0f, 0.0f, 0.0f, 0.01f);
			qf32_mul ((void*)q, (void*)q, (void*)&q0);
		}

		if (keyboard[SDL_SCANCODE_2])
		{
			component_quaternion * q = ecs_get_mut (world, e3, component_quaternion, NULL);
			component_quaternion q0;
			qf32_xyza ((void*)&q0, 0.0f, 1.0f, 0.0f, 0.01f);
			qf32_mul ((void*)q, (void*)q, (void*)&q0);
		}

		if (keyboard[SDL_SCANCODE_3])
		{
			component_quaternion * q = ecs_get_mut (world, e3, component_quaternion, NULL);
			component_quaternion q0;
			qf32_xyza ((void*)&q0, 0.0f, 0.0f, 1.0f, 0.01f);
			qf32_mul ((void*)q, (void*)q, (void*)&q0);
		}

		/*
		glUniform1i (uniform_texture1, 0);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) gcam.mvp);
		glDrawArrays (GL_TRIANGLES, 0, img.cap * CSC_GLIMAGE_VERTS_COUNT);
		*/


		ecs_progress(world, 0);
		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_GL_DeleteContext (context);
	SDL_DestroyWindow (window);
	SDL_Quit();
	ecs_fini(world);
	return 0;
}
