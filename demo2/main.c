#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_sdlglew.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"


#define main_glattr_pos 0
#define main_glattr_col 1
#define main_glattr_tex 2


void fill_texture (uint8_t index)
{
	int width = 100;
	int height = 100;
	uint8_t * data = NULL;
	unsigned size = width*height*4*sizeof(uint8_t);
	data = calloc (size, 1);
	/*
	for (int x = 0; x < width; ++x)
	for (int y = 0; y < height; ++y)
	{
		uint8_t * p = data + (x*4) + (y*width*4);
		p[0] = 0;
		p[1] = 0;
		p[2] = 0;
		p[3] = 0;
		p[index] = 255;
	}
	*/

	int x = width/2;
	int y = height/2;
	int dx = 0;
	int dy = 0;
	for (unsigned n = 0; n < 100; ++n)
	{
		x += CLAMP (dx, -2, 2);
		y += CLAMP (dy, -2, 2);
		dx += (rand() % 3) - 1;
		dy += (rand() % 3) - 1;
		if (x < 0 || x >= width){dx = -dx/2;}
		if (y < 0 || y >= height){dy = -dy/2;}
		x = CLAMP (x, 0, width-1);
		y = CLAMP (y, 0, height-1);
		int i = (x*4) + (y*width*4);
		ASSERT (i >= 0);
		ASSERT (i < size);
		data[i + index] = 255;
		//data[i + 1] = 255;
		//data[i + 2] = 255;
		//data[i + 3] = 255;
	}

	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap (GL_TEXTURE_2D);
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


	char const * shaderfiles[] = {CSC_SRCDIR"shader.glvs", CSC_SRCDIR"shader.glfs", NULL};
	GLuint glprogram = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (glprogram, main_glattr_pos, "pos" );
	glBindAttribLocation (glprogram, main_glattr_col, "col" );
	glBindAttribLocation (glprogram, main_glattr_tex, "tex" );
	glLinkProgram (glprogram);
	glUseProgram (glprogram);
	GLuint uniform_mvp = glGetUniformLocation (glprogram, "mvp");
	GLuint uniform_texture1 = glGetUniformLocation (glprogram, "texture1");

	float const p[] =
	{
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f, -1.0f, 0.0f, 1.0f, // right, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f, -1.0f, 0.0f, 1.0f, // left, bottom
	 1.0f,  1.0f, 0.0f, 1.0f, // right, top
	-1.0f,  1.0f, 0.0f, 1.0f  // left, top
	};
	float const c[] =
	{
	0.0f, 1.0f, 1.0f, 1.0f, // left, bottom
	0.0f, 1.0f, 1.0f, 1.0f, // right, bottom
	0.0f, 1.0f, 1.0f, 1.0f, // right, top
	1.0f, 1.0f, 0.0f, 1.0f, // left, bottom
	1.0f, 1.0f, 0.0f, 1.0f, // right, top
	1.0f, 1.0f, 0.0f, 1.0f  // left, top
	};
	float const t[] =
	{
	0.0f, 0.0f, // left, bottom
	1.0f, 0.0f, // right, bottom
	1.0f, 1.0f, // right, top
	0.0f, 0.0f, // left, bottom
	1.0f, 1.0f, // right, top
	0.0f, 1.0f  // left, top
	};


	GLuint vbop;
	GLuint vboc;
	GLuint vbot;
	GLuint vao;

	glGenVertexArrays (1, &vao);
	glGenBuffers (1, &vbop);
	glGenBuffers (1, &vboc);
	glGenBuffers (1, &vbot);

	glBindVertexArray (vao);

	glBindBuffer (GL_ARRAY_BUFFER, vbop);
	glBufferData (GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_pos);

	glBindBuffer (GL_ARRAY_BUFFER, vboc);
	glBufferData (GL_ARRAY_BUFFER, sizeof(c), c, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_col);

	glBindBuffer (GL_ARRAY_BUFFER, vbot);
	glBufferData (GL_ARRAY_BUFFER, sizeof(t), t, GL_STATIC_DRAW);
	glVertexAttribPointer (main_glattr_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_tex);

	GLuint textures[2];
	glGenTextures (2, textures);


	glActiveTexture (GL_TEXTURE0 + 0);
	glBindTexture (GL_TEXTURE_2D, textures[0]);
	// set the texture wrapping parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	fill_texture (0);

	glActiveTexture (GL_TEXTURE0 + 1);
	glBindTexture (GL_TEXTURE_2D, textures[1]);
	// set the texture wrapping parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	fill_texture (2);





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


		float mvp[4*4];
		float model[4*4];

		m4f32_identity (model);
		m4f32_translation_xyz (model, 1.0f, 1.0f, 0.0f);
		m4f32_mul (mvp, gcam.mvp, model);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) mvp);
		glUniform1i (uniform_texture1, 0);
		glDrawArrays (GL_TRIANGLES, 0, 6);

		m4f32_identity (model);
		m4f32_translation_xyz (model, -1.0f, -1.0f, 0.0f);
		m4f32_mul (mvp, gcam.mvp, model);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) mvp);
		glUniform1i (uniform_texture1, 1);
		glDrawArrays (GL_TRIANGLES, 0, 6);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_GL_DeleteContext (context);
	SDL_DestroyWindow (window);
	SDL_Quit();
	return 0;
}
