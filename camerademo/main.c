#include <SDL2/SDL.h>
#include <stdio.h>
#include <GL/glew.h>

#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_sdlglew.h"

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Camera Demo"

#define MAIN_RUNNING UINT32_C(0x00000001)
#define MAIN_FULLSCREEN UINT32_C(0x00000002)

enum main_glattr
{
	main_glattr_pos,
	main_glattr_col
};




int main (int argc, char * argv[])
{
	ASSERT (argc);
	ASSERT (argv);
	uint32_t main_flags = MAIN_RUNNING;
	SDL_Window * window;
	SDL_GLContext context;

	csc_sdlglew_create_window (&window, &context, WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);

	const char * sharefiles[] = {CSC_SRCDIR"shader.glvs", CSC_SRCDIR"shader.glfs", NULL};
	GLuint glprogram = csc_gl_program_from_files (sharefiles);
	glBindAttribLocation (glprogram, main_glattr_pos, "pos" );
	glBindAttribLocation (glprogram, main_glattr_col, "col" );
	glLinkProgram (glprogram);
	glUseProgram (glprogram);
	GLuint uniform_mvp = glGetUniformLocation (glprogram, "mvp");

	glDisable(GL_DEPTH_TEST);
	glClearColor (0.5, 0.0, 0.0, 0.0 );
	glViewport (0, 0, WIN_W, WIN_H);

	GLuint vao;
	GLuint vbo[2];
	glGenVertexArrays (1, &vao);
	glGenBuffers (2, vbo);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (main_glattr_pos);
	glEnableVertexAttribArray (main_glattr_col);

	//Define vertex data format for GPU:
	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_pos]);
	glVertexAttribPointer (main_glattr_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_col]);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//Defined a square of two triangles:
	const GLfloat vertex_pos[] =
	{
	0.0f,     0.0f,     0.0f,
	1.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,

	0.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,
	0.0f,     1.0f,     0.0f
	};

	//Set color of each cornder of triangles:
	const GLfloat vertex_col[] =
	{
	1, 1, 0, 1,
	0, 1, 0, 1,
	0, 0, 1, 1,

	1, 1, 0, 1,
	0, 0, 1, 1,
	1, 1, 1, 1,
	};

	//Store triangle positions and colors in GPU:
	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_pos]);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertex_pos), vertex_pos, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_col]);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertex_col), vertex_col, GL_STATIC_DRAW);

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
			glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) (gcam.mvp));
		}

		glClearColor (0.1f, 0.1f, 0.1f, 0.0f);
		glClear (GL_COLOR_BUFFER_BIT);

		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, 6);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
