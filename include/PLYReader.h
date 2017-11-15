#ifndef _PLYReader_
#define _PLYReader_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VecLib.h"

typedef struct Face
{
  int v[3];
} Face;


Vec4* readFile(const char* filename, Vec4* vertexArray, int* numVertices);
void getSizes(FILE* fp, int* verts, int* faces);


#endif
