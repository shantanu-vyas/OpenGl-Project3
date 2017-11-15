#include "PLYReader.h"

/*Only works with triangular meshes */

Vec4* readFile(const char* filename, Vec4* vertexArray, int* numVertices)
{
  FILE* fp;
  fp = fopen(filename,"r");

  int numVert;
  int numFaces;
  Vec4* vertices;
  Face* faces;

  getSizes(fp,&numVert,&numFaces);

  vertices = malloc(numVert*sizeof(Vec4));
  faces = malloc(numFaces*sizeof(Face));
  vertexArray = malloc(3*sizeof(Vec4)*numFaces);
  *numVertices = 3*numFaces;

  char line[256];
  while (fgets(line, sizeof(line), fp))
    {
      if (strstr(line, "end_header") != NULL)
	{
	  for (int i = 0; i < numVert; i++)
	    {
	      Vec4 vec;
	      fgets(line, sizeof(line), fp);
	      sscanf(line, "%f %f %f", &(vec.x),&(vec.y),&(vec.z));
	      vec.w = 1;
	      vertices[i] = vec;
	    }
	  for (int i = 0; i < numFaces; i++)
	    {
	      Face face;
	      int polySize;
	      fgets(line, sizeof(line), fp);
	      sscanf(line, "%d %d %d %d",&polySize,&(face.v[0]),&(face.v[1]),&(face.v[2]));
	      faces[i] = face;
	    }
	  break;
	}
    }
  for (int i = 0; i < numFaces; i++)
    {
      for (int p = 0; p < 3; p++)
	{
	  vertexArray[3*i+p] = vertices[faces[i].v[p]];
	}
    }
  
  free(vertices);
  free(faces);
  fclose(fp);
  return vertexArray;
}

void getSizes(FILE* fp, int* verts, int* faces)
{
  char line[256];
  while (fgets(line, sizeof(line), fp))
    {
      if (strstr(line, "element vertex") != NULL)
        {
	  sscanf(line, "element vertex %d", verts);
	  //	  printf("%d\n",*verts);
        }
      else if (strstr(line, "element face") != NULL)
        {
	  sscanf(line, "element face %d", faces);
	  return;
        }
    }
}
