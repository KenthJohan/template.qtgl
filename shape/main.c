#include <SDL2/SDL.h>
#include <stdio.h>
#include <glad.h>

#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdlcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"

#include "shaper.h"

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Camera Demo"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)

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
	SDL_Init (SDL_INIT_VIDEO);
	window = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		fprintf (stderr, "Could not create SDL_Window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext context = SDL_GL_CreateContext (window);
	if (context == NULL)
	{
		fprintf (stderr, "Could not create SDL_GLContext: %s\n", SDL_GetError());
		return 1;
	}
	gladLoadGL();

	char const * shaderfiles[] = {"../shape/shader.glvs", "../shape/shader.glfs", NULL};
	GLuint shader_program = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (shader_program, main_glattr_pos, "pos" );
	glBindAttribLocation (shader_program, main_glattr_col, "col" );
	glLinkProgram (shader_program);
	glUseProgram (shader_program);

	glEnable (GL_DEPTH_TEST);

	GLuint vao;
	GLuint vbo;
	glGenVertexArrays (1, &vao);
	glGenBuffers (1, &vbo);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (main_glattr_pos);
	glEnableVertexAttribArray (main_glattr_col);

	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, pos));

	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, sizeof (struct vertex), (void*)offsetof (struct vertex, col));



	struct vertex_handler vhandler;
	vhandler.vcapacity = 100;
	vhandler.scapacity = 10;
	vertexbatch_init (&vhandler);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, vhandler.vcapacity * sizeof (struct vertex), NULL, GL_STATIC_DRAW);
	struct vertex_transformation vtransformation;
	v4f32_set_xyzw (vtransformation.p, 0.0f, 0.0f, 0.0f, 0.0f);
	qf32_identity (vtransformation.q);
	shape_square_make (&vtransformation, &vhandler, 0);
	v4f32_set_xyzw (vtransformation.p, 4.0f, 0.0f, 0.0f, 0.0f);
	qf32_identity (vtransformation.q);
	shape_square_make (&vtransformation, &vhandler, 0);



	GLuint uniform_mvp = glGetUniformLocation (shader_program, "mvp");


	struct csc_sdlcam cam;
	csc_sdlcam_init (&cam);

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
						cam.w = w;
						cam.h = h;
						glViewport (0, 0, w, h);
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


		float r[3];
		float q[4];
		r[0] = keyboard [SDL_SCANCODE_KP_1];
		r[1] = keyboard [SDL_SCANCODE_KP_2];
		r[2] = keyboard [SDL_SCANCODE_KP_3];
		qf32_axis_angle (q, r, 0.1f);
		qf32_mul (vtransformation.q, vtransformation.q, q);
		qf32_normalize (vtransformation.q, vtransformation.q);

		//If rotation is non zero then rotate:
		if (vf32_sum (3, r))
		{
			v4f32_set_xyzw (vtransformation.p, vtransformation.q[3]*5.0f, 0.0f, 0.0f, 0.0f);
			shape_square_make (&vtransformation, &vhandler, 2);
		}

		cam.d [0] = 0.1f*(keyboard [SDL_SCANCODE_A] - keyboard [SDL_SCANCODE_D]);
		cam.d [1] = 0.1f*(keyboard [SDL_SCANCODE_LCTRL] - keyboard [SDL_SCANCODE_SPACE]);
		cam.d [2] = 0.1f*(keyboard [SDL_SCANCODE_W] - keyboard [SDL_SCANCODE_S]);
		cam.d [3] = 0;
		cam.pitch += 0.01f * (keyboard [SDL_SCANCODE_DOWN] - keyboard [SDL_SCANCODE_UP]);
		cam.yaw += 0.01f * (keyboard [SDL_SCANCODE_RIGHT] - keyboard [SDL_SCANCODE_LEFT]);

		float mvp[4*4];
		csc_sdlcam_build_matrix (&cam, mvp);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) mvp);

		glClearColor (0.1f, 0.1f, 0.1f, 0.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, 12);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
