#include "csc/csc_crossos.h"


#define NNG_STATIC_LIB
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
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


#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "Camera Demo"

#define MAIN_RUNNING    UINT32_C (0x00000001)
#define MAIN_FULLSCREEN UINT32_C (0x00000002)
#define ADDRESS_PUB "ipc:///glshape_pub"
#define ADDRESS_SUB "ipc:///glshape_sub"


static void APIENTRY openglCallbackFunction(
GLenum source,
GLenum type,
GLuint id,
GLenum severity,
GLsizei length,
const GLchar* message,
const void* userParam
){
  (void)source; (void)type; (void)id;
  (void)severity; (void)length; (void)userParam;
  fprintf(stderr, "%s\n", message);
  if (severity==GL_DEBUG_SEVERITY_HIGH) {
	  fprintf(stderr, "Aborting...\n");
	abort();
  }
}


enum main_drawobj
{
	MAIN_DRAWOBJ_SQUARE1,
	MAIN_DRAWOBJ_SQUARE2,
	MAIN_DRAWOBJ_MOUSELINE,
	MAIN_DRAWOBJ_COUNT
};


void proj (SDL_Window * window, struct csc_sdlcam * cam, float world[4])
{
	int x;
	int y;
	SDL_GetMouseState (&x, &y);
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);

	float clip[4];
	clip[0] = 2.0f * ((float)x / (float)w) - 1.0f;
	clip[1] = 1.0f - 2.0f * ((float)y / (float)h);
	clip[2] = -1.0f;
	clip[3] = 1.0f;
	//printf ("(%i %i) (%f %f)\n", x, y, clip[0], clip[1]);


	// 4D Eye (Camera) Coordinates
	float p[4*4];
	float eye[4] = {0.0f};
	m4f32_perspective1 (p, cam->fov, cam->w/cam->h, cam->n, cam->f); //Create perspective matrix
	m4f32_glu_inv (p, p);
	mv4f32_macc (eye, p, clip);
	eye[2] = -1.0f;
	eye[3] = 0.0f;

	// 4D World Coordinates
	float view[4*4];
	m4f32_mul (view, cam->mr, cam->mt);
	m4f32_glu_inv (view, view);
	v4f32_set1 (world, 0.0f);
	mv4f32_macc (world, view, eye);
	v4f32_normalize (world, world);
}


/*
glm::vec3 CFreeCamera::CreateRay() {
	// these positions must be in range [-1, 1] (!!!), not [0, width] and [0, height]
	float mouseX = getMousePositionX() / (getWindowWidth()  * 0.5f) - 1.0f;
	float mouseY = getMousePositionY() / (getWindowHeight() * 0.5f) - 1.0f;

	glm::mat4 proj = glm::perspective(FoV, AspectRatio, Near, Far);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f), CameraDirection, CameraUpVector);

	glm::mat4 invVP = glm::inverse(proj * view);
	glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
	glm::vec4 worldPos = invVP * screenPos;

	glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

	return dir;
}
*/

void proj1 (SDL_Window * window, struct csc_sdlcam * cam, float world[4])
{
	int x;
	int y;
	SDL_GetMouseState (&x, &y);
	int w;
	int h;
	SDL_GetWindowSize (window, &w, &h);

	float clip[4];
	clip[0] = 2.0f * ((float)x / (float)w) - 1.0f;
	clip[1] = 2.0f * ((float)y / (float)h) - 1.0f;
	clip[1] *= -1;
	clip[2] = 1.0f;
	clip[3] = 1.0f;
	//printf ("(%i %i) (%f %f)\n", x, y, clip[0], clip[1]);
	//printf (V4F32_FORMAT, V4F32_ARGS (clip));
	m4f32_glu_inv (cam->mvp, cam->mvp);
	mv4f32_mul (world, cam->mvp, clip);
	v4f32_normalize (world, world);
}



int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
	ASSERT (argc);
	ASSERT (argv);

	nng_socket pub;
	{
		int r;
		r = nng_pub0_open (&pub);
		NNG_EXIT_ON_ERROR (r);
		r = nng_dial (pub, ADDRESS_PUB, NULL, 0);
		//r = nng_listen (pub, ADDRESS, NULL, 0);
		NNG_EXIT_ON_ERROR (r);
	}

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

	char const * shaderfiles[] = {"../shape/shader.glvs", "../shape/shader.glfs", NULL};
	GLuint shader_program = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (shader_program, main_glattr_pos, "pos" );
	glBindAttribLocation (shader_program, main_glattr_col, "col" );
	glLinkProgram (shader_program);
	glUseProgram (shader_program);

	glEnable (GL_DEPTH_TEST);

	// Enable the debug callback
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

	struct gmeshes gm;
	gmeshes_init (&gm, 100);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_SQUARE1, 6, GL_TRIANGLES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_SQUARE2, 6, GL_TRIANGLES);
	gmeshes_allocate (&gm, MAIN_DRAWOBJ_MOUSELINE, 2, GL_LINES);
	gmeshes_square (&gm, MAIN_DRAWOBJ_SQUARE1);
	gm.flags[MAIN_DRAWOBJ_SQUARE1] |= MESH_SHOW;
	gmeshes_square (&gm, MAIN_DRAWOBJ_SQUARE2);
	gm.flags[MAIN_DRAWOBJ_SQUARE2] |= MESH_SHOW;



	GLuint uniform_mvp = glGetUniformLocation (shader_program, "mvp");
	struct csc_sdlcam cam;
	csc_sdlcam_init (&cam);


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

				default:
					break;
				}
				break;
			};
		}

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

		//Camera:
		cam.d [0] = 0.1f*(keyboard [SDL_SCANCODE_A] - keyboard [SDL_SCANCODE_D]);
		cam.d [1] = 0.1f*(keyboard [SDL_SCANCODE_LCTRL] - keyboard [SDL_SCANCODE_SPACE]);
		cam.d [2] = 0.1f*(keyboard [SDL_SCANCODE_W] - keyboard [SDL_SCANCODE_S]);
		cam.d [3] = 0;
		cam.pitchd = 0.01f * (keyboard [SDL_SCANCODE_DOWN] - keyboard [SDL_SCANCODE_UP]);
		cam.yawd = 0.01f * (keyboard [SDL_SCANCODE_RIGHT] - keyboard [SDL_SCANCODE_LEFT]);
		cam.rolld = 0.01f * (keyboard [SDL_SCANCODE_Q] - keyboard [SDL_SCANCODE_E]);
		csc_sdlcam_build (&cam);

		//Send camera info to network
		if (cam.pitchd || cam.yawd || cam.rolld || vf32_sum (3, cam.d))
		{
			//m4f32_print(cam.mr, stdout);
			int r;
			//printf ("nng_send %f\n", cam.mvp[0]);
			r = nng_send (pub, cam.mr, sizeof (float)*4*4, 0);
			NNG_EXIT_ON_ERROR (r);
		}


		gmeshes_draw (&gm, uniform_mvp, cam.mvp);

		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
