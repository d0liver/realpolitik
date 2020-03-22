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
/*
 * graph.c
 *
 * routines to make and handle resolution graph
 *
 */

#include <stdio.h>
#include <string.h>
#include "MapData.h"
#include "Unit.h"
#include "Order.h"
#include "Graph.h"
#include "Game.h"
#include "Fails.h"

GraphStruct graph[MAX_SECT+2];

/* prototypes */
void addorders(Orderlist olist);
void checknodes();
GraphNode convneigh (GraphNode start, GraphNode curr);
void doarmybfs (short nodeindex);
void dofleetbfs (short nodeindex);
void convoybfs(short start);
void tracesea (short rindex, char mark);
int diffsupp (short movetype, short supptype);

void newgraph()
{
//	graph = (Graph) NewPtr(sizeof(struct graphnode)*(NUM_SECT+2)); 
}

void deletegraph()
{
/*	if (graph != NULL)
		DisposPtr(graph);
	graph = NULL; 
*/
}

void makegraph (Orderlist *orders, short ign_units)
{
     register short i;

     initializegraph(ign_units);

     if (orders != NULL)
	  for (i = 1; i <= NUM_COUN; i++)
	       addorders(orders[i]);

     checknodes();

}

void initializegraph (short ign_units)
{
     register short i, reg;
     Sector rp;
     GraphNode gp;
     Unit up;

     /* clear the graph */
     memset(graph, 0, sizeof(GraphStruct)*(NUM_SECT+1));

     /* set sector pointers */
     for (gp = startnode(), rp = startsector(map); issector(rp); nextsector(rp), nextnode(gp)) 
     {
	  	setgsector(gp, rp);
	  	setgtype(gp, O_UNKN);
	  	gp->order = NULL;
     }

     /* assume that all units are holding, initially */
     if (!ign_units) 
     {
	  	for (i = 1; i <= NUM_COUN; i++) 
	  	{
	       for (up = startunit(units[i]); isunit(up); nextunit(up)) 
	       {
		    	reg = usector(up);
		    	gp = getnode(reg);
		    	setgowner(gp, i);
		    	setgtype(gp, O_HOLD);
		    	incgstrength(gp);
		    	incgbounce(gp);
		    	setgresolution(gp, R_NONE);
	       }
	  	}
     }

     graph[NUM_SECT+1].type = -1;
}

void 
clearmarks ()
{
     register short i;

     for (i = 1; i <= NUM_SECT; i++)
	  graph[i].mark = 0;

}

/* add orders to the graph */
void 
addorders (Orderlist olist)
{
     register Order op;
     register short startreg;

     for (op = startorder(olist); isorder(op); nextorder(op)) 
     {
	  	if (obadsyntax(op)) 
	       	continue;
	       
	  	if (otype(op) == O_PENDING || otype(op) == O_WAIVE)
		 	continue;
		 
	  	startreg = ostart(op);
	  	if (startreg == S_NONE)
	  		continue;
	  
	  	/* is there already an order for that sector? */
	  	if (graph[startreg].order) 
	  	{
	  	   	if (otype(op) != O_BUILD && otype(op) != O_REMOVE && otype(op) != O_DEF_REM)
	       		graph[startreg].resolution = R_DUPLIC;
	       	setofails(op, R_DUPLIC);
	       	continue;
	  	}
	  
	  	/* add order */
	  	addnode(op, graph+startreg);
	  
	}
}

void 
checknodes () 
{
     register GraphNode gn, dest;
     register Order op, sub;

     for (gn = startnode(); isnode(gn); nextnode(gn)) {
      if (gfails(gn))
      	   continue;
	  op = gorder(gn);
	  switch (gtype(gn)) 
	  {
	  case O_HOLD:
	       break;
		    
	  case O_MOVE:
	       /* is this unit being convoyed? */
	       if ((ounit(op) != U_ARM || !iscoastal(gsector(gn))
                      || !iscoastal(gsector(gdest(gn))) || !canconvoy(gn, gn))
	       /* can we get there from here? */
		    && !neighborsector(ounit(op), ostart(op), oscoast(op),
			       ofinish(op), ofcoast(op), 0))
			{
				gstrength(gn) = 0;
		    	setgresolution(gn, R_FAILS);
		    }
		    	
		   /* is the sector icebound? */
		   else if ((season == S_FALL || season == S_AUTUMN) && 
		   			(isice(gsector(gn)) || isice(gsector(gdest(gn)))))
		   		setgresolution(gn, R_FAILS);
		   
		   /* travelling from isthmus to isthmus has strength 0 w/o support */
		   else if (isisthmus(gsector(gn)) && isisthmus(gsector(gdest(gn))))
		   	    decgstrength(gn);
	       break;

	  case O_SUPP:
	       sub = osuborder(op);
	       if (otype(sub) != O_MOVE) 
	       {
		    	/* can we get to holding sector? */
		    	if (!neighborsector(ounit(op), ostart(op), oscoast(op),
				    ostart(sub), oscoast(sub), 1)) 
				{
			 		setgresolution(gn, R_FAILS);
			 		continue;
		    	}
	       }
	       
		   /* is the sector icebound? */
		   else if ((season == S_FALL || season == S_AUTUMN) && 
		   			(isice(gsector(gn)) || isice(gsector(gdest(gn))) || isice(gsector(gspec(gn))))
		   			)
		   {
		   		setgresolution(gn, R_FAILS);
		   		continue;
		   }
		   
		   /* does the support cross a low passibility border? */
		   else if ( isisthmus(gsector(gn)) && 
		   			 ( isisthmus(gsector(gdest(gn))) || isisthmus(gsector(gspec(gn))) ) )
		   {
		   		setgresolution(gn, R_FAILS);
		   		continue;
		   }
		   
	       else 
	       {
		    	/* can we get to move's destination? */
		    	if (!neighborsector(ounit(op), ostart(op), oscoast(op),
				    	ofinish(sub), ofcoast(sub), 1)) 
				{
			 		setgresolution(gn, R_FAILS);
			 		continue;
		    	}
		   }

	       /* now find moves that match supports */
	       dest = getnode(ostart(sub));
	       /* country in the move doesn't match the country of the unit */
	       if (ocountry(sub) != S_NONE && ocountry(sub) != gowner(dest)) {
		    setgresolution(gn, R_NSU);
		    continue;
	       }
	       /* match coasts */
	       if (oscoast(sub) != C_NONE 
		   && oscoast(sub) != oscoast(gorder(dest))) {
		    setgresolution(gn, R_NSU);
		    continue;
	       }
	       if (gtype(dest) == O_MOVE)
		    if ((ofcoast(sub) != C_NONE 
			     || ofcoast(gorder(dest)) != C_NONE)
			 && ofcoast(sub) != ofcoast(gorder(dest))) {
			 setgresolution(gn, R_NSO);
			 continue;
		    }
	       /* order for unit is different from support given */
	       if (diffsupp(gtype(dest), otype(sub))) {
		    if (gtype(dest) == O_MOVE) 
			 setgresolution(gn, R_OTM);
		    else
			 setgresolution(gn, R_NSO);
		    continue;
	       }
	       if (!gtype(dest) 
		   || (gtype(dest) == O_MOVE && gdest(dest) != getnode(ofinish(sub)))) {
		    setgresolution(gn, R_NSO);
		    continue;
	       }
		/* add to the supported unit's strength */
		/* Prevent support of illegal moves. */
           if (gtype(dest) != O_MOVE || !gfails(dest))
	           incgstrength(dest);
	       break;		    
	       
	  case O_CONV:
	       /* is it a coastal sector (or even a land sector)? */
	       if (island(gsector(gn)) && !isconvoyable(gsector(gn))) 
	       {
		    	setgresolution(gn, R_FAILS);
		    	continue;
	       }
		   /* is the sector icebound? */
		   else if ( (season == S_FALL || season == S_AUTUMN) && isice(gsector(gn)) )
		   {
		   		setgresolution(gn, R_FAILS);
		   		continue;
		   }
		   
       /* is there a corresponding move? */
	       dest = gdest(gn);
	       sub = osuborder(gorder(gn));
	       /* country in the move doesn't match the country of the unit */
	       if (ocountry(sub) != S_NONE && ocountry(sub) != gowner(dest)) {
		    setgresolution(gn, R_NSU);
		    continue;
	       }
	       if (gtype(dest) == O_UNKN || gtype(dest) != O_MOVE
		   || getnode(ofinish(sub)) != gdest(dest)) {
		    setgresolution(gn, R_NSO);
		    continue;
	       }
	       break;
		    
	  default:
	       break;
	  }
     }
}


/* returns 0 if supp can support 'move' */
/* returns 1 if not */
int diffsupp (short movetype, short supptype)
{
     switch (movetype) {
     case O_MOVE:
	  if (supptype == O_MOVE)
	       return 0;
	  else 
	       return 1;
	  break;

     case O_HOLD:
     case O_CONV:
     case O_SUPP:
     case O_NMR:
	  if (supptype == O_HOLD)
	       return 0;
	  else
	       return 1;
	  break;

     default:
	  return 1;
	  break;
     }

}

int addnode (Order op, struct graphnode *node)
{
     node->type = op->type;
     node->spec = NULL;
     switch (op->type) {
     case O_HOLD:
	  node->dest = NULL;
	  break;
     case O_MOVE:
	  node->dest = &(graph[op->finish]);
	  break;
     case O_SUPP:
     case O_CONV:
	  node->dest = &(graph[op->suborder->start]);
	  node->spec = &(graph[op->suborder->finish]);
	  break;
     default:
	  break;
     }

     node->owner = ocountry(op);
     node->sector = getsector(map, ostart(op));
     node->order = op;
     node->strength = 1;
     node->resolution = ofails(op);   
     node->mark = 0;

	 return 1;
}


/* returns true if unit at start can get to its goal through curr */
/* uses DFS, would be better if used BFS (shorter path) */
int canconvoy (GraphNode start,	GraphNode curr)
{
     GraphNode next;
     
	if (gtype(curr) == O_CONV)
		setgmark(curr, 1);
	while (isnode(next = convneigh(start, curr))) 
	{
		// if at end
		if (next == gdest(start))
		{
			setgspec(curr, next);
			return 1;
		}
		else if (gtype(next) == O_CONV && !gfails(next)) 
		{
			if (canconvoy(start, next)) 
			{
				setgspec(curr, next);
				return 1;
			}
		}
		else 
		{	
			setgspec(curr, next);
			return 1;
		}
     }

     /* if we're at the top of the tree */
     if (curr == start) 
	 {
	  	/* if we can get there via normal means */
	  	if (neighborsector(U_ARM, gindex(start), C_NONE, 
			     gindex(gdest(start)), C_NONE, 0)) 
	  	{
	       	setgspec(curr, NULL);
	       	return 1;
	  	}
	 }
	       
     return 0;
}

/* returns neighbor to i, suitable to convoying unit at s */
GraphNode convneigh (GraphNode start, GraphNode curr)
{
     Neighbor lp;
     GraphNode next;

     for (lp = startneighbor(coast1neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
     {
	  	next = getnode(nsector(lp));
	  	/* got to destination! */
	  	if (next == gdest(start) && curr != start)
	       	return next;

	  	/* been here before or no route through */
	  	if (gmarked(next) || gfails(next)) 
	       	continue;

	  	if (gtype(next) == O_CONV && gdest(next) == start 
	  		&& (gspec(next) == gdest(start) || gtype(gspec(next)) == O_CONV)
	  		)
	       	return next;	       	
     }

     /* check the second coast */
     if (isbicoastal(gsector(curr)))
	  for (lp = startneighbor(coast2neigh(gsector(curr))); isneighbor(lp); nextneighbor(lp)) 
	  {
	       next = getnode(nsector(lp));
	       if (next == gdest(start) && curr != start)
		    return next;

	       /* been here before or no route through */
	       if (gmarked(next) || gfails(next)) 
		    continue;
	       
	       if (gtype(next) == O_CONV && gdest(next) == start 
	  		&& (gspec(next) == gdest(start) || gtype(gspec(next)) == O_CONV)
			 )
		    return next;
     }

     return NULL;
}

void convoybfs(short start)
{
     GraphNode gp;
     Neighbor  np;
     short covered;
     char  markindex;
 
     clearmarks();
     gp = getnode(start);
     setgmark(gp, 1);
     covered = 0;
     markindex = 1;
     
     while (!covered) 
     {
	  covered = 1;
	  for (gp = startnode(); isnode(gp); nextnode(gp)) 
	  {
	       if (gmark(gp) == markindex) 
	       {
		    /* look at land-based neighbors first */
		    np = gp->sector->land;
		    for (np = gp->sector->land; isneighbor(np); nextneighbor(np)) 
		    {
			 if (!gmarked(getnode(nsector(np)))) 
			 {
			      setgmark(getnode(nsector(np)), markindex+1);
			      covered = 0;
			 }
		    }
		    for (np = gp->sector->sea1; isneighbor(np); nextneighbor(np)) 
		    {
			 if (!gmarked(getnode(nsector(np))) 
			     && gtype(getnode(nsector(np))) != O_UNKN) 
			 {
			      setgmark(getnode(nsector(np)), markindex);
			      tracesea(nsector(np), markindex);
			      covered = 0;
			 }
		    }
		    for (np = gp->sector->sea2; isneighbor(np); nextneighbor(np)) 
		    {
			 if (!gmarked(getnode(nsector(np))) 
			     && gtype(getnode(nsector(np))) != O_UNKN)
			 {
			      setgmark(getnode(nsector(np)), markindex);
			      tracesea(nsector(np), markindex);
			      covered = 0;
			 }
		    }
	       }
	  }
	  markindex++;
     }

}


int isnode (GraphNode x)
{
     return ((x) != NULL && ((x)->type != -1));
}

short armydistance (Unit up, short countryindex)
{
     short distance, count;

     if (!island(getsector(map,usector(up))))
	 return -1;

     doarmybfs(usector(up));

     distance = NUM_SECT+1;
     for (count = 0; count < countries[countryindex].numCenters; count++) {
	  if (countries[countryindex].home[count] != CN_NONE
	   && gmark(getnode(countries[countryindex].home[count])) < distance)
	       distance = gmark(getnode(countries[countryindex].home[count]));
     }
     if (distance == NUM_SECT+1)
	 return -1;

     return distance-1;
     
}

void doarmybfs (short nodeindex)
{
     GraphNode gp;
     Neighbor  np;
     short covered;
     char  markindex;
 
     clearmarks();
     gp = getnode(nodeindex);
     setgmark(gp, 1);
     covered = 0;
     markindex = 1;
     
     while (!covered) {
	  covered = 1;
	  for (gp = startnode(); isnode(gp); nextnode(gp)) {
	       if (gmark(gp) == markindex) {
		    /* look at land-based neighbors first */
		    np = gp->sector->land;
		    for (np = gp->sector->land; isneighbor(np); nextneighbor(np)) {
			 if (!gmarked(getnode(nsector(np)))) {
			      setgmark(getnode(nsector(np)), markindex+1);
			      covered = 0;
			 }
		    }
		    for (np = gp->sector->sea1; isneighbor(np); nextneighbor(np)) {
			 if (!gmarked(getnode(nsector(np))) 
			     && gtype(getnode(nsector(np))) != O_UNKN) {
			      setgmark(getnode(nsector(np)), markindex);
			      tracesea(nsector(np), markindex);
			      covered = 0;
			 }
		    }
		    for (np = gp->sector->sea2; isneighbor(np); nextneighbor(np)) {
			 if (!gmarked(getnode(nsector(np))) 
			     && gtype(getnode(nsector(np))) != O_UNKN) {
			      setgmark(getnode(nsector(np)), markindex);
			      tracesea(nsector(np), markindex);
			      covered = 0;
			 }
		    }
	       }
	  }
	  markindex++;
     }

}


short fleetdistance (Unit up, short countryindex)
{
     short distance, count;
     
     if (!issea(getsector(map,usector(up))))
	 return -1;

     dofleetbfs(usector(up));

     distance = NUM_SECT+1;
     for (count = 0; count < countries[countryindex].numCenters; count++) 
     {
	 if (countries[countryindex].home[count] != CN_NONE
	     && gmark(getnode(countries[countryindex].home[count])) < distance && gmark(getnode(countries[countryindex].home[count])) > 0)
	     distance = gmark(getnode(countries[countryindex].home[count]));
     }
     if (distance == NUM_SECT+1)
	 return -1;

     return distance-1;
}

void dofleetbfs (short nodeindex)
{
    GraphNode gp;
    Neighbor  np;
    short covered;
    char  markindex;
    short sea1ok, sea2ok;
    
    clearmarks();
    gp = getnode(nodeindex);
    if (island(gp->sector) && !issea(gp->sector))
	return;
    setgmark(gp, 1);
    covered = 0;
    markindex = 1;
    
    while (!covered) {
	covered = 1;
	for (gp = startnode(); isnode(gp); nextnode(gp)) {
	    if (gmark(gp) == markindex) 
	    {
		if (isbicoastal(gp->sector) && gmark(gp) != 1) 
		/* need to check which coasts we can legally explore */
		{
		    sea1ok = sea2ok = 0;
		    for (np = gp->sector->sea1; isneighbor(np); nextneighbor(np)) 
		    {
			if (gmark(getnode(nsector(np))) == markindex-1) 
			{
			    sea1ok = 1;
			    break;
			}
		    }
		    for (np = gp->sector->sea2; isneighbor(np); nextneighbor(np)) 
		    {
			if (gmark(getnode(nsector(np))) == markindex-1) 
			{
			    sea2ok = 1;
			    break;
			}
		    }
		}
		else 
		    sea1ok = sea2ok = 1;
		
		if (sea1ok) 
		    for (np = gp->sector->sea1; isneighbor(np); nextneighbor(np)) {
			if (!gmarked(getnode(nsector(np)))) 
			{ 
/*			    && gtype(getnode(nsector(np))) != O_UNKN) { */
			    setgmark(getnode(nsector(np)), markindex+1);
			    covered = 0;
			}
		    }
		if (sea2ok)
		    for (np = gp->sector->sea2; isneighbor(np); nextneighbor(np)) {
			if (!gmarked(getnode(nsector(np))))
			{
/*			    && gtype(getnode(nsector(np))) != O_UNKN) { */
			    setgmark(getnode(nsector(np)), markindex+1);
			    covered = 0;
			}
		    }
	    }
	}
	markindex++;
    }
    
}

void tracesea (short rindex, char mark)
{
     GraphNode gp, nn;
     Neighbor np;

     gp = getnode(rindex);
     if (island(gsector(gp)))
	  return;

     for (np = gp->sector->sea1; isneighbor(np); nextneighbor(np)) {
	  nn = getnode(nsector(np));
	  if (!gmarked(nn)) {
	       if (iscoastal(gsector(nn))) 
		    setgmark(nn, mark+1);
	       else if (issea(gsector(nn)) && gtype(nn) != O_UNKN) {

		    setgmark(nn, mark);
		    tracesea(nsector(np), mark);
	       }
	  }
     }
}
