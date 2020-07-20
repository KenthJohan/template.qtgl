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

//#define WIN_X SDL_WINDOWPOS_CENTERED_DISPLAY(1)
//#define WIN_Y SDL_WINDOWPOS_CENTERED_DISPLAY(1)

#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Graphics server"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)
#define MAIN_BORDERLESS UINT32_C (0x00000004)//SDL_WINDOW_BORDERLESS

#define POINTC_W 320
#define POINTC_H 20




#define VOX_XN 60
#define VOX_YN 30
#define VOX_ZN 10
#define VOX_I(x,y,z) ((z)*VOX_XN*VOX_YN + (y)*VOX_XN + (x))
#define VOX_SCALE 0.15f

#define IMG_XN 20
#define IMG_YN 120
#define IMG_CN 4
#define TEX_FORMAT GL_RGBA

enum main_glprogram
{
	MAIN_GLPROGRAM_STANDARD = 1,
	MAIN_GLPROGRAM_POINTCLOUD,
	MAIN_GLPROGRAM_VOXEL,
	MAIN_GLPROGRAM_LINE,
	MAIN_GLPROGRAM_COUNT
};

enum main_glpbo
{
	MAIN_GLPBO_0 = 1,
	MAIN_GLPBO_1,
	MAIN_GLPBO_COUNT,
};

enum main_gltex
{
	MAIN_GLTEX_0 = 1,
	MAIN_GLTEX_BW,
	MAIN_GLTEX_RGBA256,
	MAIN_GLTEX_COUNT
};

enum main_glvao
{
	MAIN_GLVAO_POINTCLOUD = 1,
	MAIN_GLVAO_GROUND,
	MAIN_GLVAO_LINES,
	MAIN_GLVAO_COUNT
};

enum main_glvbo
{
	MAIN_GLVBO_POINTCLOUD_POS,
	MAIN_GLVBO_POINTCLOUD_COL,
	MAIN_GLVBO_LINES_POS,
	MAIN_GLVBO_COUNT
};

enum main_nngsock
{
	MAIN_NNGSOCK_POINTCLOUD_POS = 1,
	MAIN_NNGSOCK_POINTCLOUD_COL,
	MAIN_NNGSOCK_PLANE,
	MAIN_NNGSOCK_TEX,
	MAIN_NNGSOCK_VOXEL,
	MAIN_NNGSOCK_GROUND,
	MAIN_NNGSOCK_LINE_POS,
	MAIN_NNGSOCK_LINE_COL,
	MAIN_NNGSOCK_COUNT
};








int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
	ASSERT (argc);
	ASSERT (argv);


	uint32_t main_flags = MAIN_RUNNING;
	SDL_Window * window;
	SDL_Init (SDL_INIT_VIDEO);
	window = SDL_CreateWindow (WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
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
	//glPointSize (10.0f);
	glLineWidth (5.0f);


	struct rendering_context ctx = {0};
	ctx.pbo_cap = MAIN_GLPBO_COUNT;
	ctx.tex_cap = MAIN_GLTEX_COUNT;
	ctx.vao_cap = MAIN_GLVAO_COUNT;
	ctx.vbo_cap = 1;
	ctx.program_cap = MAIN_GLPROGRAM_COUNT;
	rendering_context_init (&ctx);
	ctx.program_files[MAIN_GLPROGRAM_POINTCLOUD] = "../demo1/pointcloud.glvs;../demo1/pointcloud.glfs";
	ctx.program_files[MAIN_GLPROGRAM_LINE]       = "../demo1/line.glvs;../demo1/line.glfs";
	ctx.program_files[MAIN_GLPROGRAM_STANDARD]   = "../demo1/standard.glvs;../demo1/standard.glfs";
	ctx.program_files[MAIN_GLPROGRAM_VOXEL]      = "../demo1/voxel.glvs;../demo1/voxel.glfs";
	rendering_context_program_compile (&ctx);


	struct dd_img3d img_voxel;
	struct dd_img2d img_chess;
	struct dd_img2d img_0;

	img_voxel.w = VOX_XN;
	img_voxel.h = VOX_YN;
	img_voxel.d = VOX_ZN;

	img_chess.w = 2;
	img_chess.h = 2;
	img_chess.type = GL_UNSIGNED_BYTE;
	img_chess.format = GL_RGBA;

	img_0.w = IMG_XN;
	img_0.h = IMG_YN;
	img_0.type = GL_UNSIGNED_BYTE;
	img_0.format = GL_RGBA;



	uint8_t bw[4*4] =
	{
	0x88, 0x88, 0x88, 0xAA,   0x55, 0x55, 0x55, 0xAA,
	0x55, 0x55, 0x55, 0xAA,   0x88, 0x88, 0x88, 0xAA,
	};
	dd_img2d_gl_allocate_tex (&img_chess, ctx.tex[MAIN_GLTEX_BW], bw);
	dd_img2d_gl_allocate_tex (&img_0, ctx.tex[MAIN_GLTEX_0], NULL);
	dd_img2d_gl_allocate_pbo (&img_0, ctx.pbo[MAIN_GLPBO_0], NULL);
	dd_img2d_gl_allocate_pbo (&img_0, ctx.pbo[MAIN_GLPBO_1], NULL);

	nng_socket sock[MAIN_NNGSOCK_COUNT];


	struct demo_mesh_rectangle mesh_groundprojection = {0};
	mesh_groundprojection.cap = 6;
	mesh_groundprojection.uniform_mvp = glGetUniformLocation (ctx.program[MAIN_GLPROGRAM_STANDARD], "mvp");
	mesh_groundprojection.program = ctx.program[MAIN_GLPROGRAM_STANDARD];
	mesh_groundprojection.texture = ctx.tex[MAIN_GLTEX_0];
	demo_mesh_rectangle_init (&mesh_groundprojection, 1.0f);
	m4f32_scale_xyz (mesh_groundprojection.model, (float)IMG_XN/20.0f, (float)IMG_YN/20.0f, 1.0f);
	m4f32_translation_xyz (mesh_groundprojection.model, 0.0f, 0.0f, 0.0f);

	struct demo_mesh_rectangle mesh_chess = {0};
	mesh_chess.cap = 6;
	mesh_chess.uniform_mvp = glGetUniformLocation (ctx.program[MAIN_GLPROGRAM_STANDARD], "mvp");
	mesh_chess.program = ctx.program[MAIN_GLPROGRAM_STANDARD];
	mesh_chess.texture = ctx.tex[MAIN_GLTEX_BW];
	demo_mesh_rectangle_init (&mesh_chess, 10.0f);
	m4f32_scale (mesh_chess.model, 10.0f);
	m4f32_translation_xyz (mesh_chess.model, 0.0f, 0.0f, 0.0f);



	struct demo_mesh_pointcloud mpointcloud = {0};
	mpointcloud.cap = POINTC_W*POINTC_H;
	mpointcloud.uniform_mvp = glGetUniformLocation (ctx.program[MAIN_GLPROGRAM_POINTCLOUD], "mvp");
	mpointcloud.program = ctx.program[MAIN_GLPROGRAM_POINTCLOUD];
	demo_mesh_pointcloud_init (&mpointcloud);


	struct demo_mesh_voxel mvoxel = {0};
	mvoxel.cap = VOX_XN*VOX_YN*VOX_ZN;
	mvoxel.uniform_mvp = glGetUniformLocation (ctx.program[MAIN_GLPROGRAM_VOXEL], "mvp");
	mvoxel.program = ctx.program[MAIN_GLPROGRAM_VOXEL];
	mvoxel.texture_pallete = ctx.tex[MAIN_GLTEX_RGBA256];
	demo_mesh_voxel_init (&mvoxel);


	struct demo_mesh_lines mlines = {0};
	mlines.cap = 18;
	mlines.program = ctx.program[MAIN_GLPROGRAM_LINE];
	mlines.uniform_mvp = glGetUniformLocation (ctx.program[MAIN_GLPROGRAM_LINE], "mvp");
	demo_mesh_lines_init (&mlines);




	uint8_t voxel[VOX_XN*VOX_YN*VOX_ZN] = {0};
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
	mesh_voxel_update (&mvoxel, voxel, VOX_XN, VOX_YN, VOX_ZN);
	m4f32_scale (mvoxel.model, VOX_SCALE);
	m4f32_translation_xyz (mvoxel.model, 0.0f*VOX_SCALE, -(VOX_YN/2)*VOX_SCALE, -(VOX_ZN/2)*VOX_SCALE);



	pair_listen (sock + MAIN_NNGSOCK_POINTCLOUD_POS, "tcp://:9002");
	pair_listen (sock + MAIN_NNGSOCK_POINTCLOUD_COL, "tcp://:9003");
	//pair_listen (sock + MAIN_NNGSOCK_PLANE, "tcp://:9003");
	pair_listen (sock + MAIN_NNGSOCK_TEX, "tcp://:9004");
	pair_listen (sock + MAIN_NNGSOCK_VOXEL, "tcp://:9005");
	pair_listen (sock + MAIN_NNGSOCK_LINE_POS, "tcp://:9006");
	pair_listen (sock + MAIN_NNGSOCK_LINE_COL, "tcp://:9007");

	uint8_t rgb256[MESH_VOXEL_PALLETE_WHC];
	for (int i = 0; i < MESH_VOXEL_PALLETE_WHC; ++i)
	{
		rgb256[i] = rand();
	}
	demo_mesh_voxel_texture_pallete (ctx.program[MAIN_GLPROGRAM_VOXEL], ctx.tex[MAIN_GLTEX_RGBA256], rgb256);


	struct gl_mesh meshes[1];
	meshes[0].vao = ctx.vao[MAIN_GLVAO_POINTCLOUD];
	meshes[0].program = ctx.vao[MAIN_GLPROGRAM_POINTCLOUD];
	m4f32_identity (meshes[0].model);
	meshes[0].draw_from = 0;
	meshes[0].draw_to = 0;
	meshes[0].draw_mode = GL_POINTS;




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


			case SDL_WINDOWEVENT:
				switch (Event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
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
			cam.d [0] = 0.01f*(keyboard [SDL_SCANCODE_A] - keyboard [SDL_SCANCODE_D]);
			cam.d [1] = 0.01f*(keyboard [SDL_SCANCODE_LCTRL] - keyboard [SDL_SCANCODE_SPACE]);
			cam.d [2] = 0.01f*(keyboard [SDL_SCANCODE_W] - keyboard [SDL_SCANCODE_S]);
			cam.d [3] = 0;
			cam.pitchd = 0.01f * (keyboard [SDL_SCANCODE_DOWN] - keyboard [SDL_SCANCODE_UP]);
			cam.yawd = 0.01f * (keyboard [SDL_SCANCODE_RIGHT] - keyboard [SDL_SCANCODE_LEFT]);
			cam.rolld = 0.01f * (keyboard [SDL_SCANCODE_E] - keyboard [SDL_SCANCODE_Q]);
			csc_sdlcam_build (&cam);
		}


		glClearColor (0.2f, 0.3f, 0.3f, 1.0f);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		dd_img2d_gl_copy_pbo_tex (&img_0, ctx.pbo[MAIN_GLPBO_0], ctx.tex[MAIN_GLTEX_0]);


		//rendering_group_draw (&ctx, group, 1);

		demo_mesh_pointcloud_draw (&mpointcloud, cam.mvp);
		//demo_mesh_voxel_draw (&mvoxel, cam.mvp);
		demo_mesh_lines_draw (&mlines, cam.mvp);
		demo_mesh_rectangle_draw (&mesh_chess, cam.mvp);
		demo_mesh_rectangle_draw (&mesh_groundprojection, cam.mvp);


		net_recv (sock[MAIN_NNGSOCK_POINTCLOUD_POS], GL_ARRAY_BUFFER, mpointcloud.vbop, mpointcloud.cap*4*sizeof(float), 0);
		net_recv (sock[MAIN_NNGSOCK_POINTCLOUD_COL], GL_ARRAY_BUFFER, mpointcloud.vboc, mpointcloud.cap*sizeof(uint32_t), 0);
		net_recv (sock[MAIN_NNGSOCK_LINE_POS], GL_ARRAY_BUFFER, mlines.vbop, mlines.cap*4*sizeof(float), 0);
		net_recv (sock[MAIN_NNGSOCK_LINE_COL], GL_ARRAY_BUFFER, mlines.vboc, mlines.cap*sizeof(uint32_t), 0);
		//net_recv (sock[MAIN_NNGSOCK_PLANE], GL_ARRAY_BUFFER, mrectangletex.vbop, mrectangletex.cap, 0);
		//net_recv (sock[MAIN_NNGSOCK_TEX], GL_PIXEL_UNPACK_BUFFER, pbo[MAIN_GLPBO_0], TEX_W*TEX_H*4, NET_RECV_DISCARD);
		//demo_mesh_voxel_update_from_socket (&mvoxel, VOX_XN, VOX_YN, VOX_ZN, sock[MAIN_NNGSOCK_VOXEL]);

		net_update_texture (sock[MAIN_NNGSOCK_TEX], ctx.pbo[MAIN_GLPBO_0], IMG_XN*IMG_YN*IMG_CN);


		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
