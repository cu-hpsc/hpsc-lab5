// for loading graphs into memory

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

unsigned int edgelist_to_adjlist(unsigned int **save_offsets, unsigned int **save_neighbors, char *path, unsigned int reflected)
{
  // assumes single listing of unordered edges
  char buffer[128];
  unsigned int maxverts = 256;
  unsigned int *offsets = (unsigned int*) calloc(maxverts,sizeof(unsigned int));
  unsigned int *temp;
  unsigned int verts = 0;
  unsigned int *neighbors;
  unsigned int ind;

  FILE * fp;
  fp = fopen(path, "r");
  unsigned int edgeA;
  unsigned int edgeB;
  // allocation pass
  while (fgets(buffer, sizeof(buffer), fp)) {
    if (sscanf(buffer,"%d\t%d",&edgeA,&edgeB)==2) {
      verts = MAX(verts,MAX(edgeA,edgeB)+1);
      // reallocate memory if necessary
      if (verts+1 > maxverts) {
	temp = offsets;
	offsets = (unsigned int*) calloc(verts*2+1,sizeof(unsigned int));
	memcpy(offsets,temp,sizeof(unsigned int)*maxverts);
	free(temp);
	maxverts = verts*2+1;
      }
      ++offsets[edgeA+1];
      if (! reflected) {
	++offsets[edgeB+1];
      }
    }
    while (buffer[strlen(buffer)-1] != '\n'
	   && fgets(buffer, sizeof(buffer), fp)) {
      // noop
    }
  }
  // accumulate indexes
  for (ind=2; ind<verts+1; ++ind) {
    offsets[ind] += offsets[ind-1];
  }
  // save offsets
  *save_offsets = (unsigned int*) malloc(sizeof(unsigned int) * (verts+1));
  memcpy(*save_offsets,offsets,sizeof(unsigned int)*(verts+1));

  // allocate neighbors
  neighbors = (unsigned int*) malloc(sizeof(unsigned int) * offsets[verts]*2);
  // storage pass
  fseek(fp,0,SEEK_SET);
  while (fgets(buffer, sizeof(buffer), fp)) {
    if (sscanf(buffer,"%d\t%d",&edgeA,&edgeB)==2) {
      neighbors[offsets[edgeA]] = edgeB;
      ++offsets[edgeA];
      if (! reflected) {
	neighbors[offsets[edgeB]] = edgeA;
	++offsets[edgeB];
      }
    }
    while (buffer[strlen(buffer)-1] != '\n'
	   && fgets(buffer, sizeof(buffer), fp)) {
      // noop
    }
  }
  // save neighbors
  *save_neighbors = neighbors;

  // clean up incremented indexes
  free(offsets);
  return verts;
}
