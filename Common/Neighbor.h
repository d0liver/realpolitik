//---------------------------------------------------------------------------------
// Copyright (C) 2001  James M. Van Verth
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Clarified Artistic License.
// In addition, to meet copyright restrictions you must also own at 
// least one copy of any of the following board games: 
// Diplomacy, Deluxe Diplomacy, Colonial Diplomacy or Machiavelli.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Clarified Artistic License for more details.
//
// You should have received a copy of the Clarified Artistic License
// along with this program; if not, write to the Open Source Initiative
// at www.opensource.org
//---------------------------------------------------------------------------------
#ifndef _NeighborDefs_
#define _NeighborDefs_

/* base definitions for lists */

#include "Unit.h"

struct neighnode {
     short sector;          /* sector index */
     CoastType coast;           /* coastal adjacency (NSEW or none) */
     struct neighnode *next;
};
typedef struct neighnode *Neighbor, *NeighborList;

#define initnlist(lp) lp = NULL;

void allocneighbor();
void deallocneighbor();
Neighbor newneighbor();
void disposeneighbor(Neighbor np);

int addneighbor(NeighborList *lp, short sector, CoastType coast);
int deleteneighbor(NeighborList *lp, short s);
void cleanneighbors(NeighborList *lp);
void copyneighbors( NeighborList* copy, NeighborList original );

int innlist (NeighborList lp, short sector, CoastType coast, short igncoast);
Neighbor getneighbor (NeighborList lp, short sector, CoastType coast, short igncoast);
/* void printneighbor();
void printnlist(); */
void neighbor2string(char *s, Neighbor lp);
void neighbor2lstring(char *s, Neighbor lp);
void string2nlist(char *s, NeighborList *lp);
void nlist2string(char *s, NeighborList lp);
void nlist2lstring(char *s, NeighborList lp);

#define startneighbor(l) (l)
#define isneighbor(i) ((i) != NULL)
#define nextneighbor(i) ((i) = (i)->next)

#define nsector(i) ((i)->sector)
#define ncoast(i) ((i)->coast)

#endif