#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdlcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_debug_nng.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>

#include <nng/nng.h>
#include <nng/protocol/pair0/pair.h>
#include <nng/supplemental/util/platform.h>

#include "demo1.h"

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)

#define POINTC_W 320
#define POINTC_H 20

#define TEX_W 100
#define TEX_H 100


#define VOX_XN 60
#define VOX_YN 30
#define VOX_ZN 10
#define VOX_I(x,y,z) ((z)*VOX_XN*VOX_YN + (y)*VOX_XN + (x))
#define VOX_SCALE 0.15f


enum main_glprogram
{
	MAIN_GLPROGRAM_STARNDARD,
	MAIN_GLPROGRAM_POINTCLOUD,
	MAIN_GLPROGRAM_VOXEL,
	MAIN_GLPROGRAM_COUNT
};

enum main_glpbo
{
	MAIN_GLPBO_0,
	MAIN_GLPBO_1,
	MAIN_GLPBO_COUNT,
};

enum main_gltex
{
	MAIN_GLTEX_0,
	MAIN_GLTEX_BW,
	MAIN_GLTEX_RGBA256,
	MAIN_GLTEX_COUNT
};

enum main_nngsock
{
	MAIN_NNGSOCK_POINTCLOUD,
	MAIN_NNGSOCK_PLANE,
	MAIN_NNGSOCK_TEX,
	MAIN_NNGSOCK_VOXEL,
	MAIN_NNGSOCK_GROUND,
	MAIN_NNGSOCK_COUNT
};


void updatePixels (uint8_t a[], uint32_t n, uint64_t k)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		a[i] = rand();
	}
}

int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
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
	glewExperimental = 1;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}

	// Enable the debug callback
	glEnable (GL_DEBUG_OUTPUT);
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback (openglCallbackFunction, NULL);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
	glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);//Disable all messages
	glDebugMessageControl (GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);//Enable error messages


	glEnable (GL_DEPTH_TEST);
	glEnable (GL_PROGRAM_POINT_SIZE_EXT);
	glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_POINT_SMOOTH);
	//glDisable (GL_DITHER);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize (50.0f);
	glLineWidth (20.0f);

	GLuint pbo[MAIN_GLPBO_COUNT];
	GLuint gprogram[MAIN_GLPROGRAM_COUNT];
	GLuint gtexture[MAIN_GLTEX_COUNT];
	nng_socket sock[MAIN_NNGSOCK_COUNT];

	glGenBuffers (MAIN_GLPBO_COUNT, pbo);
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_0]);
	glBufferData (GL_PIXEL_UNPACK_BUFFER, TEX_W*TEX_H*4, 0, GL_STREAM_DRAW);
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_1]);
	glBufferData (GL_PIXEL_UNPACK_BUFFER, TEX_W*TEX_H*4, 0, GL_STREAM_DRAW);
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);


	char const * shaderfiles1[] = {"../demo1/pointcloud.glvs", "../demo1/pointcloud.glfs", NULL};
	gprogram[MAIN_GLPROGRAM_POINTCLOUD] = csc_gl_program_from_files (shaderfiles1);
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_POINTCLOUD], main_glattr_pos, "pos" );
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_POINTCLOUD], main_glattr_col, "col" );
	glLinkProgram (gprogram[MAIN_GLPROGRAM_POINTCLOUD]);


	char const * shaderfiles2[] = {"../demo1/standard.glvs", "../demo1/standard.glfs", NULL};
	gprogram[MAIN_GLPROGRAM_STARNDARD] = csc_gl_program_from_files (shaderfiles2);
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_STARNDARD], main_glattr_pos, "pos" );
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_STARNDARD], main_glattr_col, "col" );
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_STARNDARD], main_glattr_tex, "tex" );
	glLinkProgram (gprogram[MAIN_GLPROGRAM_STARNDARD]);


	char const * shaderfiles3[] = {"../demo1/voxel.glvs", "../demo1/voxel.glfs", NULL};
	gprogram[MAIN_GLPROGRAM_VOXEL] = csc_gl_program_from_files (shaderfiles3);
	glBindAttribLocation (gprogram[MAIN_GLPROGRAM_VOXEL], main_glattr_pos, "pos" );
	glLinkProgram (gprogram[MAIN_GLPROGRAM_VOXEL]);



	glGenTextures (MAIN_GLTEX_COUNT, gtexture);

	glBindTexture (GL_TEXTURE_2D, gtexture[MAIN_GLTEX_0]);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, TEX_W, TEX_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGenerateMipmap (GL_TEXTURE_2D);

	glBindTexture (GL_TEXTURE_2D, gtexture[MAIN_GLTEX_BW]);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	uint8_t bw[4*4] =
	{
	0x88, 0x88, 0x88, 0xFF,
	0x55, 0x55, 0x55, 0xFF,
	0x55, 0x55, 0x55, 0xFF,
	0x88, 0x88, 0x88, 0xFF,
	};
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, bw);
	glGenerateMipmap (GL_TEXTURE_2D);




	struct mesh_rectangle mrectangletex = {0};
	mrectangletex.cap = 6;
	mrectangletex.uniform_mvp = glGetUniformLocation (gprogram[MAIN_GLPROGRAM_STARNDARD], "mvp");
	mrectangletex.program = gprogram[MAIN_GLPROGRAM_STARNDARD];
	mrectangletex.texture = gtexture[MAIN_GLTEX_0];
	mesh_rectangle_init (&mrectangletex, 1.0f);
	m4f32_scale (mrectangletex.model, 0.1f);
	m4f32_translation_xyz (mrectangletex.model, 10.0f, 0.0f, 0.0f);

	struct mesh_rectangle mlidar = {0};
	mlidar.cap = 6;
	mlidar.uniform_mvp = glGetUniformLocation (gprogram[MAIN_GLPROGRAM_STARNDARD], "mvp");
	mlidar.program = gprogram[MAIN_GLPROGRAM_STARNDARD];
	mlidar.texture = gtexture[MAIN_GLTEX_BW];
	mesh_rectangle_init (&mlidar, 10.0f);
	m4f32_scale (mlidar.model, 10.0f);
	m4f32_translation_xyz (mlidar.model, 0.0f, 0.0f, -0.4f);



	struct mesh_pointcloud mpointcloud = {0};
	mpointcloud.cap = POINTC_W*POINTC_H;
	mpointcloud.uniform_mvp = glGetUniformLocation (gprogram[MAIN_GLPROGRAM_POINTCLOUD], "mvp");
	mpointcloud.program = gprogram[MAIN_GLPROGRAM_POINTCLOUD];
	mesh_pointcloud_init (&mpointcloud);


	struct mesh_voxel mvoxel = {0};
	mvoxel.cap = VOX_XN*VOX_YN*VOX_ZN;
	mvoxel.uniform_mvp = glGetUniformLocation (gprogram[MAIN_GLPROGRAM_VOXEL], "mvp");
	mvoxel.program = gprogram[MAIN_GLPROGRAM_VOXEL];
	mvoxel.texture_pallete = gtexture[MAIN_GLTEX_RGBA256];
	mesh_voxel_init (&mvoxel);

	uint8_t voxel[VOX_XN*VOX_YN*VOX_ZN] = {0};
	/*
	voxel[VOX_I(0,0,0)] = 1;
	voxel[VOX_I(0,0,1)] = 2;
	voxel[VOX_I(0,0,2)] = 3;
	voxel[VOX_I(0,0,3)] = 44;
	voxel[VOX_I(49,0,0)] = 54;
	voxel[VOX_I(49,19,0)] = 54;
	voxel[VOX_I(49,19,2)] = 44;
	voxel[VOX_I(49,19,9)] = 1;
	voxel[VOX_I(49,19,8)] = 200;
	voxel[VOX_I(49,19,7)] = 255;
	*/
	mesh_voxel_update (&mvoxel, voxel, VOX_XN, VOX_YN, VOX_ZN);
	m4f32_scale (mvoxel.model, VOX_SCALE);
	m4f32_translation_xyz (mvoxel.model, 0.0f*VOX_SCALE, -(VOX_YN/2)*VOX_SCALE, -(VOX_ZN/2)*VOX_SCALE);



	pair_listen (sock + MAIN_NNGSOCK_POINTCLOUD, "tcp://:9002");
	//pair_listen (sock + MAIN_NNGSOCK_PLANE, "tcp://:9003");
	//pair_listen (sock + MAIN_NNGSOCK_TEX, "tcp://:9004");
	pair_listen (sock + MAIN_NNGSOCK_VOXEL, "tcp://:9005");

	uint8_t rgb256[MESH_VOXEL_PALLETE_WHC];
	for (int i = 0; i < MESH_VOXEL_PALLETE_WHC; ++i)
	{
		rgb256[i] = rand();
	}
	mesh_voxel_texture_pallete (gprogram[MAIN_GLPROGRAM_VOXEL], gtexture[MAIN_GLTEX_RGBA256], rgb256);


	{
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_0]);
		// map the buffer object into client's memory
		// Note that glMapBuffer() causes sync issue.
		// If GPU is working with this buffer, glMapBuffer() will wait(stall)
		// for GPU to finish its job. To avoid waiting (stall), you can call
		// first glBufferData() with NULL pointer before glMapBuffer().
		// If you do that, the previous data in PBO will be discarded and
		// glMapBuffer() returns a new allocated pointer immediately
		// even if GPU is still working with the previous data.
		glBufferData (GL_PIXEL_UNPACK_BUFFER, TEX_W*TEX_H*4, 0, GL_STREAM_DRAW);
		//GLubyte* ptr = (GLubyte*)glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		GLubyte* ptr = (GLubyte*)glMapBufferRange (GL_PIXEL_UNPACK_BUFFER, 0, TEX_W*TEX_H*4, GL_MAP_WRITE_BIT);
		if(ptr)
		{
			// update data directly on the mapped buffer
			updatePixels (ptr, TEX_W*TEX_H*4, 1);
			glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer
		}
	}




	struct csc_sdlcam cam;
	csc_sdlcam_init (&cam);
	uint64_t framecounter = 0;

	const Uint8 * keyboard = SDL_GetKeyboardState (NULL);
	while (main_flags & MAIN_RUNNING)
	{
		framecounter++;
		SDL_Event Event;
		while (SDL_PollEvent (&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				main_flags &= ~MAIN_RUNNING;
				break;

			case SDL_KEYDOWN:
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
				}
			}
		}

		{
			//Camera:
			cam.d [0] = 0.1f*(keyboard [SDL_SCANCODE_A] - keyboard [SDL_SCANCODE_D]);
			cam.d [1] = 0.1f*(keyboard [SDL_SCANCODE_LCTRL] - keyboard [SDL_SCANCODE_SPACE]);
			cam.d [2] = 0.1f*(keyboard [SDL_SCANCODE_W] - keyboard [SDL_SCANCODE_S]);
			cam.d [3] = 0;
			cam.pitchd = 0.01f * (keyboard [SDL_SCANCODE_DOWN] - keyboard [SDL_SCANCODE_UP]);
			cam.yawd = 0.01f * (keyboard [SDL_SCANCODE_RIGHT] - keyboard [SDL_SCANCODE_LEFT]);
			cam.rolld = 0.01f * (keyboard [SDL_SCANCODE_Q] - keyboard [SDL_SCANCODE_E]);
			csc_sdlcam_build (&cam);
		}


		glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// copy pixels from PBO to texture object
		// Use offset instead of ponter
		glBindTexture (GL_TEXTURE_2D, gtexture[MAIN_GLTEX_0]);
		glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_0]);
		glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, TEX_W, TEX_H, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		mesh_rectangle_draw (&mrectangletex, cam.mvp);
		mesh_rectangle_draw (&mlidar, cam.mvp);

		mesh_pointcloud_draw (&mpointcloud, cam.mvp);

		mesh_voxel_draw (&mvoxel, cam.mvp);


		net_recv (sock[MAIN_NNGSOCK_POINTCLOUD], GL_ARRAY_BUFFER, mpointcloud.vbop, mpointcloud.cap, 0);
		//net_recv (sock[MAIN_NNGSOCK_PLANE], GL_ARRAY_BUFFER, mrectangletex.vbop, mrectangletex.cap, 0);
		//net_recv (sock[MAIN_NNGSOCK_TEX], GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_0], TEX_W*TEX_H*4, NET_RECV_DISCARD);
		mesh_voxel_update_from_socket (&mvoxel, VOX_XN, VOX_YN, VOX_ZN, sock[MAIN_NNGSOCK_VOXEL]);


		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
