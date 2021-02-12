#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_sdlglew.h"
#include "csc/csc_dod.h"
#include "csc/csc_glimage.h"
#include "csc/csc_glpointcloud.h"
#include "csc/csc_gltex.h"

//#include "api.h"


#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>

#include "mygl.h"








#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"




#define COMP_POS 0
#define COMP_VEL 1
#define COMP_MASS 2





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




	struct csc_glimgcontext img;
	img.cap = 4;
	img.glprogram = csc_gl_program_from_files1 (CSC_SRCDIR"image.glvs;"CSC_SRCDIR"image.glfs");
	glLinkProgram (img.glprogram);
	csc_glimage_init (&img);


	struct csc_glimg imgs[] =
	{
	{.pos = {0.0f, 0.0f, 0.0f, 0.0f}, 0.5f, 0.5f, 0},
	{.pos = {0.0f, 1.0f, 0.0f, 0.0f}, 0.5f, 0.5f, 1},
	{.pos = {1.0f, 0.0f, 0.0f, 0.0f}, 0.5f, 0.5f, 2},
	{.pos = {1.0f, 1.0f, 0.0f, 0.0f}, 0.5f, 0.5f, 3}};

	csc_glimage_update (&img, imgs, 4);



	struct csc_glpointcloud pointcloud;
	pointcloud.cap = 1000;
	pointcloud.glprogram = csc_gl_program_from_files1 (CSC_SRCDIR"pointcloud.glvs;"CSC_SRCDIR"pointcloud.glfs");
	glLinkProgram (pointcloud.glprogram);
	csc_glpointcloud_init (&pointcloud);


	struct csc_gltexcontext texctx = {0};
	texctx.cap = 2;
	texctx.tex[0].width = 256;
	texctx.tex[0].height = 256;
	texctx.tex[0].layers = 4;
	texctx.tex[0].unit = 0;
	texctx.tex[1].width = 64;
	texctx.tex[1].height = 64;
	texctx.tex[1].layers = 4;
	texctx.tex[1].unit = 0;
	csc_gltexcontext_init (&texctx);


	struct csc_gcam gcam;
	csc_gcam_init (&gcam);
	v4f32_set_xyzw (gcam.p, 0.0f, 0.0f, -4.0f, 1.0f);

	const Uint8 * keyboard = SDL_GetKeyboardState (NULL);
	while (main_flags & CSC_SDLGLEW_RUNNING)
	{
		SDL_Event event;
		while (SDL_PollEvent (&event))
		{
			csc_sdlglew_event_loop (window, &event, &main_flags, &gcam);
		}

		{
			//Control graphics camera
			csc_sdl_motion_wasd (keyboard, gcam.d);
			csc_sdl_motion_pyr (keyboard, gcam.pyrd);
			vsf32_mul (3, gcam.d, gcam.d, 0.01f);
			vsf32_mul (3, gcam.pyrd, gcam.pyrd, 0.01f);
			csc_gcam_update (&gcam);
		}

		glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		if (keyboard[SDL_SCANCODE_0])
		{
			glBindTexture (GL_TEXTURE_2D_ARRAY, texctx.tbo[0]);
		}

		if (keyboard[SDL_SCANCODE_1])
		{
			glBindTexture (GL_TEXTURE_2D_ARRAY, texctx.tbo[1]);
		}

		csc_glimage_draw (&img, gcam.mvp);
		csc_glpointcloud_draw (&pointcloud, gcam.mvp);



		/*
		glUniform1i (uniform_texture1, 0);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) gcam.mvp);
		glDrawArrays (GL_TRIANGLES, 0, img.cap * CSC_GLIMAGE_VERTS_COUNT);
		*/


		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_GL_DeleteContext (context);
	SDL_DestroyWindow (window);
	SDL_Quit();
	return 0;
}
