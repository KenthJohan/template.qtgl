#pragma once

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



#define GL_PBOINFO_MAX 10
struct gl_pboinfo
{
	uint32_t size8[GL_PBOINFO_MAX];
	uint32_t pbo[GL_PBOINFO_MAX];
};


void gl_pboinfo_init (struct gl_pboinfo * info)
{
	glGenBuffers (GL_PBOINFO_MAX, info->pbo);
}



void gl_pboinfo_allocate (struct gl_pboinfo * info, uint32_t index, uint32_t size8, void * data)
{
	printf ("gl_pboinfo_push index %jd %jd\n", (intmax_t)index, (intmax_t)size8);
	uint32_t pbo = info->pbo[index];
	if (index >= GL_PBOINFO_MAX)
	{
		printf ("gl_pboinfo_push max number of pixel buffer object reached!");
		return;
	}
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
	glBufferData (GL_PIXEL_UNPACK_BUFFER, size8, data, GL_STREAM_DRAW);
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
	info->size8[index] = size8;
}



void gl_pboinfo_nng_recv (struct gl_pboinfo * info, uint32_t index, nng_socket sock)
{
	ASSERT (index < GL_PBOINFO_MAX);
	uint32_t pbo = info->pbo[index];
	uint32_t size8 = info->size8[index];
	ASSERT (glIsBuffer (pbo));
	glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo);
	// map the buffer object into client's memory
	// Note that glMapBuffer() causes sync issue.
	// If GPU is working with this buffer, glMapBuffer() will wait(stall)
	// for GPU to finish its job. To avoid waiting (stall), you can call
	// first glBufferData() with NULL pointer before glMapBuffer().
	// If you do that, the previous data in PBO will be discarded and
	// glMapBuffer() returns a new allocated pointer immediately
	// even if GPU is still working with the previous data.
	glBufferData (GL_PIXEL_UNPACK_BUFFER, size8, 0, GL_STREAM_DRAW);
	//GLubyte* ptr = (GLubyte*)glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	GLubyte * ptr = (GLubyte*)glMapBufferRange (GL_PIXEL_UNPACK_BUFFER, 0, size8, GL_MAP_WRITE_BIT);
	ASSERT (ptr);

	/*
	for (uint32_t i = 0; i < size8; ++i)
	{
		ptr[i] = rand();
	}
	*/
	int rv;
	size_t sz = size8;
	rv = nng_recv (sock, ptr, &sz, NNG_FLAG_NONBLOCK);
	//printf ("rv %i\n", rv);
	if (rv == NNG_EAGAIN)
	{}
	else if (rv != 0)
	{
		NNG_EXIT_ON_ERROR (rv);
	}
	else
	{
		printf ("size8: %jd. recv: %jd\n", (intmax_t)size8, (intmax_t)sz);
		ASSERTF (size8 == sz, "%jd == %jd", (intmax_t)size8, (intmax_t)sz);
		for (uint32_t i = 0; i < size8; ++i)
		{
			//printf ("%02x ", ptr[i]);
		}
		//printf ("\n");

		//memcpy (ptr, val, size8);
	}
	glUnmapBuffer (GL_PIXEL_UNPACK_BUFFER);  // release pointer to mapping buffer
}

