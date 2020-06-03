#include "csc/csc_crossos.h"

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/pair0/pair.h>
#include <nng/supplemental/util/platform.h>


#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_malloc_file.h"
#include "csc/csc_sdlcam.h"
#include "csc/csc_gl.h"
#include "csc/csc_math.h"
#include "csc/csc_debug_nng.h"

#include <stdio.h>
#include <unistd.h>

#define LIDAR_W 320
#define LIDAR_H 20
#define LIDAR_WH (LIDAR_W*LIDAR_H)




struct interface_vbo
{
	char const * address;
	uint32_t vbo;
	uint32_t esize;
	uint32_t dim;
	uint32_t offset;
	uint32_t length;
};





int main (int argc, char * argv[])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);

	struct interface_vbo vboiface;
	vboiface.address = "tcp://192.168.1.176:9002";
	vboiface.vbo = 1;
	vboiface.esize = sizeof(float);
	vboiface.dim = 4;
	vboiface.offset = 0;
	vboiface.length = LIDAR_WH;


	nng_socket socket;
	{
		int r;
		r = nng_pair0_open (&socket);
		NNG_EXIT_ON_ERROR (r);
		r = nng_dial (socket, "tcp://192.168.1.176:9002", NULL, 0);
		NNG_EXIT_ON_ERROR (r);
	}


	printf ("while:\n");
	/*
	while (1)
	{
		int rv;
		size_t sz;
		float * val = NULL;
		rv = nng_recv (socket, &val, &sz, NNG_FLAG_ALLOC);
		nng_free (val, sz);
		printf ("New message %i\n", sz);
	}
	*/
	float data[LIDAR_WH*4];
	while (1)
	{
		usleep (100000);
		for (int i = 0; i < LIDAR_WH; ++i)
		{
			data[i*4 + 0] = (float)rand() / (float)RAND_MAX;
			data[i*4 + 1] = (float)rand() / (float)RAND_MAX;
			data[i*4 + 2] = (float)rand() / (float)RAND_MAX;
			data[i*4 + 3] = 1.0f;
		}
		vsf32_mul (LIDAR_WH*4, data, data, 10.0f);
		int r;
		r = nng_send (socket, data, sizeof (data), NNG_FLAG_NONBLOCK);
		if (r != NNG_EAGAIN)
		{
			NNG_EXIT_ON_ERROR (r);
		}
	}
	return 0;
}
