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


void fill_texture (uint8_t * data, int width, int height, uint32_t channels, uint32_t epoch)
{
	memset (data, 0, width * height * channels);
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
	int x = width/2;
	int y = height/2;
	int dx = 0;
	int dy = 0;
	for (uint32_t i = 0; i < epoch; ++i)
	{
		x += CLAMP (dx, -2, 2);
		y += CLAMP (dy, -2, 2);
		dx += (rand() % 3) - 1;
		dy += (rand() % 3) - 1;
		if (x < 0 || x >= width){dx = -dx/2;}
		if (y < 0 || y >= height){dy = -dy/2;}
		x = CLAMP (x, 0, width-1);
		y = CLAMP (y, 0, height-1);
		int i = (x*channels) + (y*width*channels);
		ASSERT (i >= 0);
		data[i + 0] = 255;
		//data[i + 1] = 255;
		//data[i + 2] = 255;
		//data[i + 3] = 255;
	}
}


void fill_texture2 (uint8_t * data, int w, int h, int c, uint8_t value)
{
	for (int i = 0; i < w * h * c; ++i)
	{
		data[i] = value;
	}
	/*
	for (int x = 0; x < width; ++x)
	for (int y = 0; y < height; ++y)
	{
		uint8_t * p = data + (x*channels) + (y*width*channels);
		p[0] = 0;
		p[1] = 0;
		p[2] = 0;
		p[3] = 0;
		p[index] = 255;
	}
	*/
}



#define MESHES_SQUARE 1

struct csc_meshes
{
	GLuint vbop;
	GLuint vboc;
	GLuint vbot;
	GLuint vao;
	float * vp;
	float * vc;
	float * vt;
	uint32_t vlast;
	uint32_t vcap;
	uint32_t * mi;
	uint32_t * mn;
	uint32_t mlast;
	uint32_t mcap;
};


void meshes_init (struct csc_meshes * m)
{
	m->vp = malloc (m->vcap * sizeof (float[4]));
	m->vc = malloc (m->vcap * sizeof (float[4]));
	m->vt = malloc (m->vcap * sizeof (float[2]));
	m->mi = malloc (m->mcap * sizeof (uint32_t));
	m->mn = malloc (m->mcap * sizeof (uint32_t));
	m->vlast = 0;
	m->mlast = 0;
}

void meshes_push_square (struct csc_meshes * m, float temp)
{
	m->mi [m->mlast] = m->vlast;
	m->mn [m->mlast] = 6;
	float * vp = m->vp + m->vlast*4;
	float * vc = m->vc + m->vlast*4;
	float * vt = m->vt + m->vlast*4;
	m->mlast += 1;
	m->vlast += 6;
	float const p[] =
	{
	-1.0f, -1.0f, temp, 1.0f, // left, bottom
	 1.0f, -1.0f, temp, 1.0f, // right, bottom
	 1.0f,  1.0f, temp, 1.0f, // right, top
	-1.0f, -1.0f, temp, 1.0f, // left, bottom
	 1.0f,  1.0f, temp, 1.0f, // right, top
	-1.0f,  1.0f, temp, 1.0f  // left, top
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
	memcpy (vp, p, 6 * sizeof (float[4]));
	memcpy (vc, c, 6 * sizeof (float[4]));
	memcpy (vt, t, 6 * sizeof (float[2]));
}



void meshes_update (struct csc_meshes * m)
{
	GLenum target = GL_ARRAY_BUFFER;
	GLintptr offset = 0;
	glBindBuffer (target, m->vbop);
	glBufferSubData (target, offset, m->vcap * sizeof (float[4]), m->vp);
	glBindBuffer (target, m->vboc);
	glBufferSubData (target, offset, m->vcap * sizeof (float[4]), m->vc);
	glBindBuffer (target, m->vbot);
	glBufferSubData (target, offset, m->vcap * sizeof (float[2]), m->vt);
}


void meshes_draw (uint32_t ov[], uint32_t nv[], uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
	{
		GLint first = ov[i];
		GLsizei count = nv[i];
		glDrawArrays (GL_TRIANGLES, first, count);
	}
}




void meshes_push (struct csc_meshes * m, int type)
{
	switch (type)
	{
	case MESHES_SQUARE:
		meshes_push_square (m, 0.0f);
		//meshes_push_square (m, 0.1f);
		break;
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


	char const * shaderfiles[] = {CSC_SRCDIR"shader.glvs", CSC_SRCDIR"shader.glfs", NULL};
	GLint glprogram = csc_gl_program_from_files (shaderfiles);
	glBindAttribLocation (glprogram, main_glattr_pos, "pos" );
	glBindAttribLocation (glprogram, main_glattr_col, "col" );
	glBindAttribLocation (glprogram, main_glattr_tex, "tex" );
	glLinkProgram (glprogram);
	glUseProgram (glprogram);
	GLint uniform_mvp = glGetUniformLocation (glprogram, "mvp");
	GLint uniform_texture1 = glGetUniformLocation (glprogram, "texture1");
	ASSERT (uniform_mvp >= 0);
	ASSERT (uniform_texture1 >= 0);


	GLuint vbop;
	GLuint vboc;
	GLuint vbot;
	GLuint vao;

	glGenVertexArrays (1, &vao);
	glGenBuffers (1, &vbop);
	glGenBuffers (1, &vboc);
	glGenBuffers (1, &vbot);

	//Allocate GPU memory for vertex buffer:
	glBindBuffer (GL_ARRAY_BUFFER, vbop);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float[4])*6*100, NULL, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vboc);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float[4])*6*100, NULL, GL_STATIC_DRAW);
	glBindBuffer (GL_ARRAY_BUFFER, vbot);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float[2])*6*100, NULL, GL_STATIC_DRAW);

	//Specify the data stucture of each vertex:
	glBindVertexArray (vao);
	glBindBuffer (GL_ARRAY_BUFFER, vbop);
	glVertexAttribPointer (main_glattr_pos, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_pos);
	glBindBuffer (GL_ARRAY_BUFFER, vboc);
	glVertexAttribPointer (main_glattr_col, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_col);
	glBindBuffer (GL_ARRAY_BUFFER, vbot);
	glVertexAttribPointer (main_glattr_tex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray (main_glattr_tex);


	srand (0);

	struct csc_meshes meshes = {0};
	meshes.vao = vao;
	meshes.vbop = vbop;
	meshes.vboc = vboc;
	meshes.vbot = vbot;
	meshes.vcap = 100*6;
	meshes.mcap = 10;
	meshes_init (&meshes);
	meshes_push (&meshes, MESHES_SQUARE);
	meshes_update (&meshes);


	//https://sites.google.com/site/john87connor/texture-object/tutorial-09-6-array-texture
	//https://community.khronos.org/t/when-to-use-glactivetexture/64913/2
	GLuint textures[1];
	{
		int width = 16;
		int height = 16;
		int layers = 4;
		int channels = 4;
		unsigned size = width * height * channels * sizeof(uint8_t);
		uint8_t * data = calloc (size, 1);
		glGenTextures (1, textures);
		glActiveTexture (GL_TEXTURE0 + 0);
		glBindTexture (GL_TEXTURE_2D_ARRAY, textures[0]);
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage3D (GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layers);
		//fill_texture (data, width, height, channels, 0);
		//fill_texture2 (data, width, height, channels, 255);
		fill_texture (data, width, height, channels, 10);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//fill_texture2 (data, width, height, channels, 200);
		fill_texture (data, width, height, channels, 10);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//fill_texture2 (data, width, height, channels, 100);
		fill_texture (data, width, height, channels, 10);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//fill_texture2 (data, width, height, channels, 50);
		fill_texture (data, width, height, channels, 10);
		glTexSubImage3D (GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	/*
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
	*/





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

		/*
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
		*/



		glUniform1i (uniform_texture1, 0);
		glUniformMatrix4fv (uniform_mvp, 1, GL_FALSE, (const GLfloat *) gcam.mvp);
		meshes_draw (meshes.mi, meshes.mn, meshes.mlast);


		SDL_Delay (10);
		SDL_GL_SwapWindow (window);
	}

	SDL_GL_DeleteContext (context);
	SDL_DestroyWindow (window);
	SDL_Quit();
	return 0;
}
