#include <SDL2/SDL.h>
#include <stdio.h>
#include <glad.h>

#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_lin.h"
#include "csc/csc_sdlcam.h"

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
	gladLoadGL();

	char const * shader_vertex_source = csc_malloc_file ("../camerademo/shader.glvs");
	char const * shader_fragment_source = csc_malloc_file ("../camerademo/shader.glfs");
	//puts(shader_vertex_source);
	//puts(shader_fragment_source);
	GLuint shader_vs;
	GLuint shader_fs;
	GLuint shader_program;
	shader_vs = glCreateShader (GL_VERTEX_SHADER);
	shader_fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (shader_vs, 1, (const GLchar **)&shader_vertex_source, NULL);
	glShaderSource (shader_fs, 1, (const GLchar **)&shader_fragment_source, NULL);
	glCompileShader (shader_vs);
	glCompileShader (shader_fs);
	GLint shader_status;
	glGetShaderiv (shader_vs, GL_COMPILE_STATUS, &shader_status);
	if (shader_status == GL_FALSE)
	{
		fprintf (stderr, "vertex shader compilation failed\n" );
		shader_infolog (shader_vs);
		return 1;
	}
	glGetShaderiv (shader_fs, GL_COMPILE_STATUS, &shader_status);
	if (shader_status == GL_FALSE)
	{
		fprintf (stderr, "fragment shader compilation failed\n" );
		shader_infolog (shader_fs);
		return 1;
	}
	shader_program = glCreateProgram();
	glAttachShader (shader_program, shader_vs);
	glAttachShader (shader_program, shader_fs);
	glBindAttribLocation (shader_program, main_glattr_pos, "pos" );
	glBindAttribLocation (shader_program, main_glattr_col, "col" );
	glLinkProgram (shader_program);
	glUseProgram (shader_program);

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

	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_pos]);
	glVertexAttribPointer (main_glattr_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_col]);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, 0);

	const GLfloat vertex_pos[] =
	{
	0.0f,     0.0f,     0.0f,
	1.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,

	0.0f,     0.0f,     0.0f,
	1.0f,     1.0f,     0.0f,
	0.0f,     1.0f,     0.0f
	};

	const GLfloat vertex_col[] =
	{
	1, 1, 0, 1,
	0, 1, 0, 1,
	0, 0, 1, 1,

	1, 1, 0, 1,
	0, 0, 1, 1,
	1, 1, 1, 1,
	};

	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_pos]);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertex_pos), vertex_pos, GL_STATIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, vbo[main_glattr_col]);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertex_col), vertex_col, GL_STATIC_DRAW);

	GLuint uniform_mvp = glGetUniformLocation (shader_program, "mvp");



	struct Camera camera;
	camera_init (&camera, window);


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

		camera_update (&camera, keyboard);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) (camera.mvp));

		glViewport (0, 0, WIN_W, WIN_H);
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
