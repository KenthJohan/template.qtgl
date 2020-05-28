#include "csc/csc_crossos.h"


#define NNG_STATIC_LIB
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/pair0/pair.h>
#include <nng/supplemental/util/platform.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>

#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdlcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_debug_nng.h"
#include "mesh.h"
#include "proj1.h"
#include "ray_triangle_intersect.h"


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Camera Demo"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)
#define ADDRESS_PUB "ipc:///glshape_pub"
#define ADDRESS_SUB "ipc:///glshape_sub"


static void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam)
{
	(void)source;
	(void)type;
	(void)id;
	(void)severity;
	(void)length;
	(void)userParam;
	fprintf (stderr, "%s\n", message);
	if (severity == GL_DEBUG_SEVERITY_HIGH)
	{
		fprintf (stderr, "Aborting...\n");
		abort();
	}
}


enum main_drawobj
{
	MAIN_DRAWOBJ_SQUARE1,
	MAIN_DRAWOBJ_SQUARE2,
	MAIN_DRAWOBJ_MOUSELINE,
	MAIN_DRAWOBJ_POINTCLOUD,
	MAIN_DRAWOBJ_PLANEFIT,
	MAIN_DRAWOBJ_NORMAL,
	MAIN_DRAWOBJ_COUNT
};


enum gtype
{
	GTYPE_V4F32,
	GTYPE_M4F32,
	GTYPE_VBO_VF32
};

unsigned gtype_size (enum gtype type)
{
	switch (type)
	{
	case GTYPE_V4F32: return sizeof(float)*4;
	case GTYPE_M4F32: return sizeof(float)*4*4;
	default:return 0;
	}
}

#define INTERFACE1_READ  UINT64_C (0x01)
#define INTERFACE1_WRITE UINT64_C (0x02)

struct interface1
{
	char const * name;
	char const * address;
	uint64_t flag;
	enum gtype type;
	union
	{
		void * data;
		unsigned mesh;
	};
	nng_socket socket;
};


void interface1_init (struct interface1 iface[])
{
	for (;iface->name != NULL; iface++)
	{
		int r;
		r = nng_pair0_open (&(iface->socket));
		NNG_EXIT_ON_ERROR (r);
		r = nng_listen (iface->socket, iface->address, NULL, 0);
		NNG_EXIT_ON_ERROR (r);
	}
}


void interface1_send (struct interface1 iface[])
{
	for (;iface->name != NULL; iface++)
	{
		if ((iface->flag & INTERFACE1_READ) == 0) {continue;}
		int r;
		size_t size = gtype_size (iface->type);
		r = nng_send (iface->socket, iface->data, size, NNG_FLAG_NONBLOCK);
		if (r != NNG_EAGAIN)
		{
			NNG_EXIT_ON_ERROR (r);
		}
	}
}


void interface1_recv (struct interface1 iface[], struct gmeshes * gm)
{
	for (;iface->name != NULL; iface++)
	{
		if ((iface->flag & INTERFACE1_WRITE) == 0) {continue;}
		int rv;
		size_t sz;
		float * val = NULL;


		rv = nng_recv (iface->socket, &val, &sz, NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);
		if (rv == 0)
		{
			printf ("New message %i\n", sz);

			switch (iface->type)
			{
			case GTYPE_VBO_VF32:{
				glBindBuffer (GL_ARRAY_BUFFER, gm->vbop[iface->mesh]);
				GLsizeiptr length = MIN(gm->vcap[iface->mesh] * sizeof(float) * 4, sz);
				float * v = glMapBufferRange (GL_ARRAY_BUFFER, 0, length, GL_MAP_WRITE_BIT);
				memcpy(v, val, length);
				glUnmapBuffer (GL_ARRAY_BUFFER);
			}break;
			case GTYPE_V4F32:
				memcpy (iface->data, val, sizeof (float)*4);
			break;
			}

			nng_free (val, sz);
		}
		else if (rv != NNG_EAGAIN)
		{
			NNG_EXIT_ON_ERROR (rv);
		}
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
	glPointSize (10.0f);
	glLineWidth (20.0f);

	char const * shaderfiles[] = {"../shape/shader.glvs", "../shape/shader.glfs", NULL};
	GLuint shader_program = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (shader_program, main_glattr_pos, "pos" );
	glBindAttribLocation (shader_program, main_glattr_col, "col" );
	glLinkProgram (shader_program);
	glUseProgram (shader_program);



	struct gmeshes gm;
	gmeshes_init (&gm, 100);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_SQUARE1, 6, GL_TRIANGLES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_SQUARE2, 6, GL_TRIANGLES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_MOUSELINE, 2, GL_LINES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_POINTCLOUD, 320*20, GL_POINTS);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_PLANEFIT, 6, GL_TRIANGLES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_NORMAL, 6, GL_LINES);

	gmeshes_color (&gm, MAIN_DRAWOBJ_SQUARE1, 1.0f, 0.0f, 0.0f);
	gmeshes_color (&gm, MAIN_DRAWOBJ_SQUARE2, 1.0f, 0.0f, 0.0f);
	gmeshes_color (&gm, MAIN_DRAWOBJ_MOUSELINE, 1.0f, 0.0f, 0.0f);
	gmeshes_color (&gm, MAIN_DRAWOBJ_POINTCLOUD, 1.0f, 1.0f, 1.0f);
	gmeshes_color (&gm, MAIN_DRAWOBJ_PLANEFIT, 1.0f, 1.0f, 0.0f);
	gmeshes_color (&gm, MAIN_DRAWOBJ_NORMAL, 1.0f, 0.0f, 1.0f);

	gmeshes_square (&gm, MAIN_DRAWOBJ_SQUARE1);
	gmeshes_square (&gm, MAIN_DRAWOBJ_SQUARE2);
	gmeshes_points (&gm, MAIN_DRAWOBJ_POINTCLOUD);
	gmeshes_square (&gm, MAIN_DRAWOBJ_PLANEFIT);

	gm.flags[MAIN_DRAWOBJ_SQUARE1] &= ~MESH_SHOW;
	gm.flags[MAIN_DRAWOBJ_SQUARE2] &= ~MESH_SHOW;
	gm.flags[MAIN_DRAWOBJ_POINTCLOUD] |= MESH_SHOW;
	gm.flags[MAIN_DRAWOBJ_PLANEFIT] |= MESH_SHOW;
	gm.flags[MAIN_DRAWOBJ_NORMAL] |= MESH_SHOW;

	GLuint uniform_mvp = glGetUniformLocation (shader_program, "mvp");
	struct csc_sdlcam cam;
	csc_sdlcam_init (&cam);



	struct interface1 iface[] =
	{
	{.name = "cam_mr",     .address = "tcp://:9000", .flag = INTERFACE1_READ, .type = GTYPE_M4F32, .data = cam.mr},
	{.name = "cam_mp",     .address = "tcp://:9001", .flag = INTERFACE1_READ, .type = GTYPE_M4F32, .data = cam.mp},
	{.name = "pointcloud", .address = "tcp://:9002", .flag = INTERFACE1_WRITE, .type = GTYPE_VBO_VF32, .mesh = MAIN_DRAWOBJ_POINTCLOUD},
	{.name = "planefit_p", .address = "tcp://:9003", .flag = INTERFACE1_WRITE, .type = GTYPE_V4F32, .data = gm.p + MAIN_DRAWOBJ_PLANEFIT*4},
	{.name = "planefit_q", .address = "tcp://:9004", .flag = INTERFACE1_WRITE, .type = GTYPE_V4F32, .data = gm.q + MAIN_DRAWOBJ_PLANEFIT*4},
	{.name = "normal",     .address = "tcp://:9005", .flag = INTERFACE1_WRITE, .type = GTYPE_VBO_VF32, .mesh = MAIN_DRAWOBJ_NORMAL},
	{.name = NULL}
};

	interface1_init (iface);


	const Uint8 * keyboard = SDL_GetKeyboardState (NULL);
	while (main_flags & MAIN_RUNNING)
	{
		SDL_Event Event;
		while (SDL_PollEvent (&Event))
		{
			switch (Event.type)
			{
			case SDL_QUIT:
				main_flags &= ~MAIN_RUNNING;
				break;

			case SDL_MOUSEBUTTONDOWN:break;

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

				case 't':
					gm.p[MAIN_DRAWOBJ_PLANEFIT*4] += 2.0f;
					break;

				case 'o':{
					float line0 [4] = {0.0f, 0.0f, 0.0f, 1.0f};
					float line1 [4] = {0.0f, 0.0f, 1.0f, 1.0f};
					proj1 (window, &cam, line1);
					gmeshes_line (&gm, MAIN_DRAWOBJ_MOUSELINE, line0, line1);
					gm.flags[MAIN_DRAWOBJ_MOUSELINE] |= MESH_SHOW;
					break;}

				case 'p':{
					float line0 [4] = {0.0f, 0.0f, 0.0f, 1.0f};
					float line1 [4] = {0.0f, 0.0f, 1.0f, 1.0f};
					float d[4];
					//vf32_cpy (3, line0, cam.p);
					line0[0] = -cam.p[0];
					line0[1] = -cam.p[1];
					line0[2] = -cam.p[2];
					//vf32_print(stdout, line0, 4, "%+2.2f ");
					proj2 (window, &cam, d);

					vsf32_macc (d, d, 10.0f, 3);
					v4f32_add (line1, line0, d);
					gmeshes_line (&gm, MAIN_DRAWOBJ_MOUSELINE, line0, line1);
					gm.flags[MAIN_DRAWOBJ_MOUSELINE] |= MESH_SHOW;


					float v0[4] = {-1.0f, -1.0f, 0.0f, 1.0f};
					float v1[4] = {1.0f, -1.0f, 0.0f, 1.0f};
					float v2[4] = {1.0f,  1.0f, 0.0f, 1.0f};

					//vvf32_sub (4, v0, v0, cam.p);
					//vvf32_sub (4, v1, v1, cam.p);
					//vvf32_sub (4, v2, v2, cam.p);

					float t;
					printf ("rayTriangleIntersect1 %i\n", ray_triangle_intersect (line0, d, v0, v1, v2, &t));
					printf ("rayTriangleIntersect2 %i\n", ray_triangle_intersect (cam.p, d, v0, v1, v2, &t));


					break;}

				default:
					break;
				}
				break;
			};
		}


		{
			//Rotate a mesh:
			float r[4];
			r[0] = keyboard [SDL_SCANCODE_KP_1];
			r[1] = keyboard [SDL_SCANCODE_KP_2];
			r[2] = keyboard [SDL_SCANCODE_KP_3];
			vsf32_macc (gm.p + MAIN_DRAWOBJ_SQUARE1*4, r, 0.1f, 3);
			r[0] = keyboard [SDL_SCANCODE_KP_4];
			r[1] = keyboard [SDL_SCANCODE_KP_5];
			r[2] = keyboard [SDL_SCANCODE_KP_6];
			qf32_axis_angle (r, r, vf32_sum (3, r) ? 0.1f : 0.0f);
			qf32_mul (gm.q + MAIN_DRAWOBJ_SQUARE1*4, gm.q + MAIN_DRAWOBJ_SQUARE1*4, r);
			qf32_normalize (gm.q + MAIN_DRAWOBJ_SQUARE1*4, gm.q + MAIN_DRAWOBJ_SQUARE1*4);
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



		//Send camera info to network
		if (cam.pitchd || cam.yawd || cam.rolld || vf32_sum (3, cam.d))
		{
			//m4f32_print(cam.mr, stdout);
			//int r;
			//printf ("nng_send %f\n", cam.mvp[0]);
			//r = nng_send (pub, cam.mr, sizeof (float)*4*4, 0);
			//NNG_EXIT_ON_ERROR (r);
		}

		interface1_send (iface);
		interface1_recv (iface, &gm);
		//qf32_print (gm.q + MAIN_DRAWOBJ_PLANEFIT * 4, stdout);


		gmeshes_draw (&gm, uniform_mvp, cam.mvp);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
