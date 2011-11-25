#include "lz77.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char buffer[1 << 24];
unsigned int len;

int foo_writer(void *output, unsigned char c)
{
	 buffer[len ++] = c;
}


int main(int argc, char **argv)
{
	 FILE *fin = fopen(argv[2], "rb");
	 fseek(fin, 0, SEEK_END);
	 unsigned int size = ftell(fin);
	 fseek(fin, 0, SEEK_SET);

	 char *data = (char *)malloc(size);
	 fread(data, size, 1, fin);

	 fclose(fin);

	 len = 0;
	 if (strcmp(argv[1], "x") == 0)
	 {
		  lz77_decode(data, size, &buffer[0], 1 << 24, &len);
	 }
	 else
	 {
		  struct lz77_context_t cont;
		  lz77_context_init(&cont);
		  len = 0; lz77_encode(&cont, data, size, &foo_writer, NULL);
	 }
		  
	 FILE *fou = fopen(argv[3], "wb");
	 fwrite(buffer, len, 1, fou);
	 fclose(fou);
	 
	 return 0;
}
