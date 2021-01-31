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
//#include "csc/csc_glimage.h"
#include "csc/csc_glpointcloud.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"




#define COMP_POS 0
#define COMP_VEL 1
#define COMP_MASS 2







void fill_texture (uint8_t * data, int w, int h, uint32_t c, uint32_t n)
{
	memset (data, 0, w * h * c);
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
	//ASSERT (index < channels);
	int x = w/2;
	int y = h/2;
	int dx = 0;
	int dy = 0;
	for (uint32_t i = 0; i < n; ++i)
	{
		x += CLAMP (dx, -2, 2);
		y += CLAMP (dy, -2, 2);
		dx += (rand() % 3) - 1;
		dy += (rand() % 3) - 1;
		if (x < 0 || x >= w){dx = -dx/2;}
		if (y < 0 || y >= h){dy = -dy/2;}
		x = CLAMP (x, 0, w-1);
		y = CLAMP (y, 0, h-1);
		int i = (x*c) + (y*w*c);
		ASSERT (i >= 0);
		data[i + 0] = 255;
	}
	for (uint32_t i = 0; i < n; ++i)
	{
		x += CLAMP (dx, -2, 2);
		y += CLAMP (dy, -2, 2);
		dx += (rand() % 3) - 1;
		dy += (rand() % 3) - 1;
		if (x < 0 || x >= w){dx = -dx/2;}
		if (y < 0 || y >= h){dy = -dy/2;}
		x = CLAMP (x, 0, w-1);
		y = CLAMP (y, 0, h-1);
		int i = (x*c) + (y*w*c);
		ASSERT (i >= 0);
		data[i + 1] = 255;
	}
	for (uint32_t i = 0; i < n; ++i)
	{
		x += CLAMP (dx, -2, 2);
		y += CLAMP (dy, -2, 2);
		dx += (rand() % 3) - 1;
		dy += (rand() % 3) - 1;
		if (x < 0 || x >= w){dx = -dx/2;}
		if (y < 0 || y >= h){dy = -dy/2;}
		x = CLAMP (x, 0, w-1);
		y = CLAMP (y, 0, h-1);
		int i = (x*c) + (y*w*c);
		ASSERT (i >= 0);
		data[i + 2] = 255;
	}
}


void fill_texture2 (uint8_t * data, int w, int h, int c, uint8_t value)
{
	for (int i = 0; i < w * h * c; ++i)
	{
		data[i] = value;
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


	char const * shaderfiles[] = {CSC_SRCDIR"image.glvs", CSC_SRCDIR"image.glfs", NULL};

	struct csc_glimage img;
	img.cap = 4;
	img.glprogram = csc_gl_program_from_files (shaderfiles);
	glLinkProgram (img.glprogram);
	csc_glimage_init (&img);

	float xyzw[] =
	{
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 2.0f, 0.0f,
	0.0f, 0.0f, 3.0f, 0.0f,
	};
	float wh[] =
	{
	0.5f, 0.5f,
	0.5f, 0.5f,
	0.5f, 0.5f,
	0.5f, 0.5f,
	};
	csc_glimage_update_xywh (&img, xyzw, wh, 4);



	//https://sites.google.com/site/john87connor/texture-object/tutorial-09-6-array-texture
	//https://community.khronos.org/t/when-to-use-glactivetexture/64913/2
	GLuint textures[1];
	{
		srand (0);
		int width = 256;
		int height = 256;
		int layers = 4;
		int channels = 4;
		unsigned size = width * height * channels * sizeof(uint8_t);
		uint8_t * data = calloc (size, 1);
		glGenTextures (1, textures);
		glActiveTexture (GL_TEXTURE0 + 0);
		glBindTexture (GL_TEXTURE_2D_ARRAY, textures[0]);//Depends on glActiveTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//Depends on glBindTexture()
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//Depends on glBindTexture()
		glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layers);//Depends on glBindTexture()
		//fill_texture (data, width, height, channels, 0);
		//fill_texture2 (data, width, height, channels, 255);
		fill_texture (data, width, height, channels, 200);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);//Depends on glBindTexture()
		//fill_texture2 (data, width, height, channels, 200);
		fill_texture (data, width, height, channels, 200);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);//Depends on glBindTexture()
		//fill_texture2 (data, width, height, channels, 100);
		fill_texture (data, width, height, channels, 200);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);//Depends on glBindTexture()
		//fill_texture2 (data, width, height, channels, 50);
		fill_texture (data, width, height, channels, 200);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);//Depends on glBindTexture()
	}






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

		csc_glimage_draw (&img, gcam.mvp);

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
