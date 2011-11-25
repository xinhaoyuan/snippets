#include "lz77.h"
#include <string.h>

void
lz77_context_init(struct lz77_context_t *cont)
{
	 ukkonen_context_init(&cont->uk_cont);
}

// #include <stdio.h>

void
lz77_encode(struct lz77_context_t *cont, unsigned char *data, unsigned int size,
			stream_writer writer, void *stream)
{
	 ukkonen_context_reset(&cont->uk_cont, data);
	 unsigned int i = 0;
	 while (i < size)
	 {
		  unsigned int ab_pos, ab_len;
		  ukkonen_match(&cont->uk_cont, (unsigned char *)(data + i), size - i, &ab_pos, &ab_len);

		  unsigned int l = i - ab_pos;
		  unsigned int r = ab_len;
		  unsigned char f = 0;

		  // printf("write %d %d\n", ab_pos, ab_len);

		  if (r == 0) l = 0;
		  
		  if (l & 0xffff0000)
		  {
			   if (l & 0xff000000)
					f |= 0x30;
			   else f |= 0x20;
		  }
		  else if (l & 0xff00)
			   f |= 0x10;

		  if (r & 0xffff0000)
		  {
			   if (r & 0xff000000)
					f |= 0x03;
			   else f |= 0x02;
		  }
		  else if (r & 0xff00)
			   f |= 0x01;

		  writer(stream, f);
		  while (1)
		  {
			   writer(stream, (unsigned char)l);
			   l >>= 8;
			   if (f < 0x10) break;
			   f -= 0x10;
		  }

		  while (1)
		  {
			   writer(stream, (unsigned char)r);
			   r >>= 8;
			   if (f == 0) break;
			   f -= 1;
		  }
		  
		  if ((i += ab_len) < size)
		  {
			   writer(stream, (unsigned char)data[i]);
			   // printf("write char %c\n", data[i]);
			   ++ i;
			   ++ ab_len;
		  }

		  // fgetc(stdin);

		  // printf("%d\n", i);
		  while (ab_len -- > 0)
		  {
			   ukkonen_toward_char(&cont->uk_cont);
		  }
	 }
	 // printf("suffix fix : %d\n", cont->uk_cont.suffix_link_fix_count);
}

unsigned int
lz77_decode(unsigned char *input, unsigned int input_size,
			unsigned char *output, unsigned int output_size, unsigned int *write_size)
{
	 unsigned int r = 0;
	 unsigned int i = 0;
	 unsigned int o = 0;
	 while (i < input_size)
	 {
		  unsigned char f = input[i ++];
		  unsigned int l, r;
		  switch (f >> 4)
		  {
		  case 0:
			   l = input[i];
			   break;
		  case 1:
			   l = input[i] | (input[i + 1] << 8);
			   break;
		  case 2:
			   l = input[i] | (input[i + 1] << 8) | (input[i + 2] << 16);
			   break;
		  case 3:
			   l = input[i] | (input[i + 1] << 8) | (input[i + 2] << 16) | (input[i + 3] << 24);
		  }
		  i += (f >> 4) + 1;

		  switch (f & 0xf)
		  {
		  case 0:
			   r = input[i];
			   break;
		  case 1:
			   r = input[i] | (input[i + 1] << 8);
			   break;
		  case 2:
			   r = input[i] | (input[i + 1] << 8) | (input[i + 2] << 16);
			   break;
		  case 3:
			   r = input[i] | (input[i + 1] << 8) | (input[i + 2] << 16) | (input[i + 3] << 24);
		  }
		  i += (f & 0xf) + 1;
		  // printf("get (%d, %d)\n", l, r);

		  if (o + r + (i < input_size ? 1 : 0) <= output_size)
		  {
			   memcpy(output + o, output + o - l, r);
			   if (i < input_size)
			   {
					output[o + r] = input[i];
					// printf("write %c\n", input[i]);
					
					++ i;
					o += r + 1;
			   }
			   else o += r;
		  }
		  else return r;

		  // fgetc(stdin);
		  *write_size = o;
		  r = i;
	 }
}
