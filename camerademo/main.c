#include <SDL2/SDL.h>
#include <stdio.h>
#include <GL/glew.h>

#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"

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

void shader_infolog (GLuint shader)
{
	GLint length = 0;
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &length);
	char * info = malloc (sizeof(char) * length);
	glGetShaderInfoLog (shader, length, NULL, info);
	fputs (info, stderr);
	free (info);
}


int main (int argc, char * argv[])
{
	ASSERT (argc);
	ASSERT (argv);
	uint32_t main_flags = MAIN_RUNNING;
	SDL_Window * window;
	SDL_Init (SDL_INIT_VIDEO);
	window = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		printf("Could not create SDL_Window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext (window);
	if (context == NULL)
	{
		printf("Could not create SDL_GLContext: %s\n", SDL_GetError());
		return 1;
	}
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));


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

	while (main_flags & MAIN_RUNNING)
	{
		SDL_Event Event;
		while (SDL_PollEvent (&Event))
		{
			if (Event.type == SDL_KEYDOWN)
			{
				switch (Event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					main_flags &= ~MAIN_RUNNING;
					break;

				case 'f':
					main_flags ^= MAIN_FULLSCREEN;
					if (main_flags & MAIN_FULLSCREEN)
					{
						SDL_SetWindowFullscreen (window, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else
					{
						SDL_SetWindowFullscreen (window, SDL_WINDOW_OPENGL);
					}
					if(1)
					{
						int w;
						int h;
						SDL_GetWindowSize (window, &w, &h);
						gcam.w = w;
						gcam.h = h;
						glViewport (0, 0, w, h);
					}
					break;
					break;

				default:
					break;
				}
			}
			else if (Event.type == SDL_QUIT)
			{
				main_flags &= ~MAIN_RUNNING;
			}
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
