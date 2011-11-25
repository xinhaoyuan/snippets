#ifndef __LZ77__
#define __LZ77__

#include "ukkonen.h"

struct lz77_context_t
{
	 struct ukkonen_context_t uk_cont;
};

typedef int (*stream_writer)(void *stream, unsigned char data);

void lz77_context_init(struct lz77_context_t *cont);

void lz77_encode(struct lz77_context_t *cont, unsigned char *data, unsigned int size,
				 stream_writer writer, void *stream);

unsigned int lz77_decode(unsigned char *input, unsigned int input_size,
						 unsigned char *output, unsigned int output_size, unsigned int *write_size);

#endif
