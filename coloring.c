#include <stdio.h>

#include "parse.h"
#include "graphload.h"
#include "rdtsc.h"

// very stupid hash
unsigned int verthash(unsigned int input) {
  return ((((input * 0x5DEECE66D) + 0xB) * 0x5DEECE66D) + 0xB);
}

// stores indexes of `props` that have value `check` into `storage`
// returns number of stored indexes
unsigned int compact(unsigned int *storage, unsigned int *props, unsigned int proplen, unsigned int check)
{
  unsigned int ind;
  unsigned int stored = 0;
  // LINEAR VERSION
  for (ind=0; ind<proplen; ++ind) {
    if (props[ind] == check) {
      storage[stored] = ind;
      ++stored;
    }
  }
  return stored;
}

// colors vertices from `candidates` which have minimal hash of their uncolored
// neighbors, flagging any of their neighbors as dropped.
// (updates `colorset` and `dropset`)
void colorverts(volatile unsigned int *colorset, unsigned int colors,
		volatile unsigned int *dropset, unsigned int *candidateset,
		unsigned int candidates, unsigned int *offsets,
		unsigned int *neighbors)
{
  unsigned int cind;
  unsigned int vind;
  unsigned int myhash;
  unsigned int lowest;
  unsigned int nind;
  unsigned int neighbor;
  // LINEAR VERSION
  for (cind = 0; cind < candidates; ++cind) {
    vind = candidateset[cind];
    myhash = verthash(vind);
    lowest = 1;
    for (nind = offsets[vind]; nind < offsets[vind+1]; ++nind) {
      neighbor = neighbors[nind];
      // neighbor may have been updated this round
      if ((! dropset[neighbor] && verthash(neighbors[nind]) < myhash)
	  || colorset[neighbor] == colors) {
	lowest = 0;
	break;
      }
    }
    if (lowest) {
      // color
      colorset[vind] = colors;
      // add top dropset along with neighbors
      dropset[vind] = 1;
      for (nind = offsets[vind]; nind < offsets[vind+1]; ++nind) {
	dropset[neighbors[nind]] = 1;
      }
    }
  }
}


int main(int argc, char **argv)
{
  struct args args;
  unsigned int verts;
  unsigned int *offsets;
  unsigned int *neighbors;
  unsigned int colors;
  unsigned int *colorset;
  unsigned int candidates;
  unsigned int *candidateset; // indexes
  unsigned int *dropset; // binary; 1 if unavailable to be a candidate

  ticks_t lasttick;
  unsigned int color_mine;
  unsigned int color_near;
  unsigned int vind;
  unsigned int eind;

  // argument parsing via parse.h
  parse_args(&args, argc, argv);

  // load graph
  verts = edgelist_to_adjlist(&offsets,&neighbors,args.graph_path,
			      args.reflected);
  // edges == offsets[verts]
  fprintf(stderr,"%s loaded\n",args.graph_path);

  // bookkeepping allocations
  colors = 0;
  colorset = (unsigned int *) calloc(verts,sizeof(unsigned int));
  candidateset = (unsigned int*) calloc(verts,sizeof(unsigned int));
  candidates = compact(candidateset,colorset,verts,0);
  dropset = (unsigned int*) calloc(verts,sizeof(unsigned int));

  // loop on colors
  lasttick = rdtsc();
  while (candidates) {
    ++colors;
    // Maximum (given previous removals) independent set
    while (candidates) {
      colorverts(colorset,colors,dropset,candidateset,candidates,offsets,neighbors);
      candidates = compact(candidateset,dropset,verts,0);
    }
    // regenerate candidates from uncolored verts
    // LINEAR VERSION
    for (candidates = 0; candidates < verts; ++candidates) {
      dropset[candidates] = (colorset[candidates] == 0) ? 0 : 1;
    }
    candidates = compact(candidateset,colorset,verts,0);
  }
  lasttick = rdtsc()-lasttick;
  printf("Colored in %lld ticks\n",lasttick);


  // validation
  for (vind = 0; vind < verts; ++vind) {
    color_mine = colorset[vind];
    for (eind = offsets[vind]; eind < offsets[vind+1]; ++eind) {
      color_near = colorset[neighbors[eind]];
      if (color_mine == color_near) {
	fprintf(stderr,"vertex %d and neighbor vertex %d both have color %d\n",
		vind,neighbors[eind],color_mine);
      }
    }
  }

  printf("Used %d colors\n",colors);

  free(offsets);
  free(neighbors);
  free(colorset);
  free(candidateset);
  free(dropset);
  return 0;
}
