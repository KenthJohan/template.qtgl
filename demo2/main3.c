#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdl_motion.h"
#include "csc/csc_gcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_sdlglew.h"
#include "csc/experiment/csc_glimage.h"
#include "csc/experiment/csc_glpointcloud.h"
#include "csc/experiment/csc_gltex.h"

#include "csc/experiment/netgl.h"
#include "csc/experiment/netgl_opengl.h"

#include <flecs.h>
#include <posix_set_os_api.h>



#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <GL/glew.h>
#include <stdio.h>



#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Texture Demo"


static struct csc_gcam global_gcam = {0};
static struct csc_gltexcontext global_texctx = {0};
static struct csc_glimgcontext global_imgctx = {0};
static struct csc_glpointcloud global_pointcloud = {0};
static ecs_world_t * world;

typedef struct v4f32 component_position;
typedef struct v2f32 component_wh;
typedef uint32_t component_texlayer;




ECS_COMPONENT_DECLARE (component_position);
ECS_COMPONENT_DECLARE (component_wh);
ECS_COMPONENT_DECLARE (component_texlayer);
ECS_TAG_DECLARE (tag_imgs);
ECS_TAG_DECLARE (tag_points);









#define RECTANLGE_RENDER_MAX 100
static void rectangle_render (ecs_iter_t *it)
{
	static float vertex[CSC_GLIMAGE_VERTS_COUNT * CSC_GLIMAGE_POS_DIM * RECTANLGE_RENDER_MAX] = {0.0f};
	ECS_COLUMN(it, component_position, comp_position, 1);
	ECS_COLUMN(it, component_wh, comp_wh, 2);
	ECS_COLUMN(it, component_texlayer, comp_texlayer, 3);
	csc_glimage_begin_draw (&global_imgctx, global_gcam.mvp);
	int32_t count = MIN(RECTANLGE_RENDER_MAX, it->count);
	float * v = vertex;
	for (int32_t i = 0; i < count; i ++)
	{
		float * p = comp_position[i].v;
		float * wh = comp_wh[i].v;
		//printf ("%3.3f %3.3f %3.3f, %3.3f, %3.3f, %3i\n", p[i].v[0], p[i].v[1], p[i].v[2], r[i].v[0], r[i].v[1], l[i].layer);
		csc_gl_make_rectangle_pos (v, p[0], p[1], p[2], comp_texlayer[i], wh[0], wh[1], CSC_GLIMAGE_POS_DIM);
		v += CSC_GLIMAGE_VERTS_COUNT * CSC_GLIMAGE_POS_DIM;
	}
	glBindBuffer (GL_ARRAY_BUFFER, global_imgctx.vbop);
	glBufferSubData (GL_ARRAY_BUFFER, 0, count * CSC_GLIMAGE_VERTS_COUNT * CSC_GLIMAGE_POS_DIM * sizeof (float), vertex);
	glDrawArrays (GL_TRIANGLES, 0, count * CSC_GLIMAGE_VERTS_COUNT);
}



static void pointcloud_render (ecs_iter_t *it)
{
	ECS_COLUMN(it, component_position, comp_position, 1);
	csc_glpointcloud_begin_draw (&global_pointcloud, global_gcam.mvp);
	glBindBuffer (GL_ARRAY_BUFFER, global_pointcloud.vbop);
	glBufferSubData (GL_ARRAY_BUFFER, 0, it->count * CSC_GLPOINTCLOUD_POS_DIM * sizeof (float), comp_position);
	glDrawArrays (GL_POINTS, 0, it->count);
}


static void pointcloud_onadd (ecs_iter_t *it)
{
	ECS_COLUMN(it, component_position, comp_position, 1);
	for (int32_t i = 0; i < it->count; ++i)
	{
		float * p = comp_position[i].v;
		p[0] = (((float)rand() / (float)RAND_MAX)-0.5f) * 1.0f;
		p[1] = (((float)rand() / (float)RAND_MAX)-0.5f) * 1.0f;
		p[2] = (((float)rand() / (float)RAND_MAX)-0.5f) * 1.0f;
		p[3] = 10.0f;
	}
}






#define MYPACKAGE_MAX 100000
static int mypackage_receiver (void *ptr)
{
	IPaddress ip;
	TCPsocket server;
	TCPsocket client;

	if (SDLNet_ResolveHost (&ip, NULL, 9999) == -1)
	{
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(1);
	}

	server = SDLNet_TCP_Open (&ip);

	if (!server)
	{
		printf ("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit (2);
	}


stage_connection:
	{
		client = SDLNet_TCP_Accept (server);
		if(!client)
		{
			printf ("SDLNet_TCP_Accept: %s\n", SDLNet_GetError());
			SDL_Delay(2000);
			goto stage_connection;
		}
		/* get the clients IP and port number */
		IPaddress *remoteip;
		remoteip = SDLNet_TCP_GetPeerAddress (client);
		if (!remoteip)
		{
			printf ("SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
			SDL_Delay(1000);
			goto stage_connection;
		}
		Uint32 ipaddr;
		ipaddr = SDL_SwapBE32 (remoteip->host);
		printf ("Accepted a connection from %d.%d.%d.%d port %hu\n", ipaddr >> 24,(ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff, remoteip->port);
	}


	static uint8_t buffer[10000];

	while(1)
	{
		int len = SDLNet_TCP_Recv (client, buffer, sizeof (buffer));
		if (len <= 0)
		{
			printf ("SDLNet_TCP_Recv: %s\n", SDLNet_GetError());
			SDL_Delay (1000);
			goto stage_connection;
		}

		buffer[len] = 0;
		printf ("Received: %s\n", buffer);

		ecs_query_t * query = ecs_query_new (world, "component_position, tag_points");
		ecs_iter_t it = ecs_query_iter (query);
		while (ecs_query_next(&it))
		{
			ECS_COLUMN (&it, component_position, comp_position, 1);
			for (int32_t i = 0; i < it.count; ++i)
			{
				float * p = comp_position[i].v;
				p[0] += 0.1f;
			}
		}

	}

	return 0;
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
	if (SDLNet_Init() == -1)
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}
	SDL_Thread * thread = SDL_CreateThread (mypackage_receiver, "pointcloud_receiver", (void *)NULL);

	glEnable (GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable (GL_BLEND);
	glEnable (GL_DEPTH_TEST);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	global_imgctx.cap = RECTANLGE_RENDER_MAX;
	global_imgctx.glprogram = csc_gl_program_from_files1 (CSC_SRCDIR"image.glvs;"CSC_SRCDIR"image.glfs");
	glLinkProgram (global_imgctx.glprogram);
	csc_glimage_init (&global_imgctx);



	global_pointcloud.cap = 1000;
	global_pointcloud.glprogram = csc_gl_program_from_files1 (CSC_SRCDIR"pointcloud.glvs;"CSC_SRCDIR"pointcloud.glfs");
	glLinkProgram (global_pointcloud.glprogram);
	csc_glpointcloud_init (&global_pointcloud);


	global_texctx.cap = 2;
	global_texctx.tex[0].width = 256;
	global_texctx.tex[0].height = 256;
	global_texctx.tex[0].layers = 4;
	global_texctx.tex[0].unit = 0;
	global_texctx.tex[1].width = 64;
	global_texctx.tex[1].height = 64;
	global_texctx.tex[1].layers = 4;
	global_texctx.tex[1].unit = 0;
	csc_gltexcontext_init (&global_texctx);



	csc_gcam_init (&global_gcam);
	v4f32_set_xyzw (global_gcam.p, 0.0f, 0.0f, -4.0f, 1.0f);





	world = ecs_init();
	ECS_COMPONENT_DEFINE(world, component_position);
	ECS_COMPONENT_DEFINE(world, component_wh);
	ECS_COMPONENT_DEFINE(world, component_texlayer);
	ECS_TAG_DEFINE(world, tag_imgs);
	ECS_TAG_DEFINE(world, tag_points);
	//ECS_SYSTEM(world, bounce, EcsOnUpdate, type_flatimage);
	ECS_TYPE(world, components_img, component_position, component_wh, component_texlayer, tag_imgs);
	ECS_TYPE(world, components_pointcloud, component_position, tag_points);
	ECS_SYSTEM(world, rectangle_render, EcsOnUpdate, component_position, component_wh, component_texlayer, tag_imgs);
	ECS_SYSTEM(world, pointcloud_render, EcsOnUpdate, component_position, tag_points);
	ECS_SYSTEM(world, pointcloud_onadd, EcsMonitor, component_position, tag_points);


	ecs_entity_t eimg[4];
	memcpy(eimg, ecs_bulk_new (world, components_img, 4), sizeof(eimg));
	ecs_set(world, eimg[0], component_position, {0.0f, 0.0f, 0.0f});
	ecs_set(world, eimg[0], component_wh, {1.0f, 1.0f});
	ecs_set(world, eimg[0], component_texlayer, {0});
	ecs_set(world, eimg[1], component_position, {0.0f, 1.0f, 0.0f});
	ecs_set(world, eimg[1], component_wh, {1.0f, 1.0f});
	ecs_set(world, eimg[1], component_texlayer, {1});
	ecs_set(world, eimg[2], component_position, {1.0f, 0.0f, 0.0f});
	ecs_set(world, eimg[2], component_wh, {1.0f, 1.0f});
	ecs_set(world, eimg[2], component_texlayer, {2});
	ecs_set(world, eimg[3], component_position, {1.0f, 1.0f, 0.0f});
	ecs_set(world, eimg[3], component_wh, {1.0f, 2.0f});
	ecs_set(world, eimg[3], component_texlayer, {3});
	ecs_entity_t epc[100];
	memcpy(epc, ecs_bulk_new (world, components_pointcloud, 50), sizeof(epc));
	ecs_set(world, epc[0], component_position, {0.0f, 0.0f, 0.0f});
	//ecs_query_t const *q_parent = ecs_query_new(world, "components_pointcloud");
	//ecs_iter_t it = ecs_query_iter(q_sub);
	ecs_set_target_fps (world, 60);



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


		if (keyboard[SDL_SCANCODE_0])
		{
			glBindTexture (GL_TEXTURE_2D_ARRAY, global_texctx.tbo[0]);
		}

		if (keyboard[SDL_SCANCODE_1])
		{
			glBindTexture (GL_TEXTURE_2D_ARRAY, global_texctx.tbo[1]);
		}

		if (keyboard[SDL_SCANCODE_G])
		{
			component_position * p;
			//p = ecs_get_mut(world, eimg[0], component_position, NULL);
			//p->v[0] -= 0.1f;
			p = ecs_get_mut(world, epc[0], component_position, NULL);
			p->v[0] -= 0.01f;
			p->v[3] = 100.0f;
		}

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
