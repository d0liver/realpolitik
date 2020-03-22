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
#ifndef __MapDataDefs__
#define __MapDataDefs__

#include "Neighbor.h"
#include "Unit.h"
#include "Dislodge.h"
#include "Order.h"
#include "MacTypes.h"

const int SECT_NONE	= 0;
const int MAX_SECT = 512;

/* sector type flags */
#define F_SUPP 0x1
#define F_LAND 0x2
#define F_SEA  0x4
#define F_ICE  0x8         /* ice bound in Fall */
#define F_ISTH 0x10        /* e.g. Nap <-> Sic */
#define F_CONV 0x20        /* coastal region that can be used to convoy  */

/* country numbers */
#define CN_NONE 0
#define MAX_COUN 64

#define MAX_NAMES 8

class Rgn;

struct mapnode {	
	char *names[MAX_NAMES]; /* name of region */
	Unit unit;              /* unit, if any */
	unsigned char type;		/* land, sea or coast; supply sector or not */
	CoastType coast1;   /* direction of coast1 (NSEW) */
	CoastType coast2;   /* direction of coast2 */
 	NeighborList land;      /* regions adjacent by land */
	NeighborList sea1;
	NeighborList sea2; 
	Point land_loc;         /* location of unit */
	Point coast1_loc;
	Point coast2_loc;
	Point name_loc;         /* location of name */
	Rgn* rgn;         	/* regions describing sector */
	Rgn* coast1_rgn;
	Rgn* coast2_rgn;
};
/* type has five bits:
 * 		0x1 is supply center; 0x2 is army access; 0x4 is fleet access
 *      0x8 is ice;  0x10 is isthmus
 */
 
typedef struct mapnode *Sector;

#define kHomeCenters 16

struct cnode {
	char name[32];
	char adjective[32];
	char abbreviation;
	short builds;
	short home[kHomeCenters];
	short numCenters;
	short patID;
	short colorID;
};

extern struct mapnode *map;            	/* map data */
extern struct cnode *countries;         	/* country data */
extern UnitList     *units;
extern SupplyList   *supplies;
extern Orderlist    *orders;
extern short dislodges;
extern DislodgeList dslglist;

extern short total_supply;
extern short win_supply;


extern char *scoast[5], *lcoast[5];         /* coastnames */

extern short NUM_COUN;
extern short NUM_SECT;


bool			AllocMapData(void);
bool			ReadMapData(void);
void			CleanMapData(void);
void			SectorToPoint(Point* thePoint, short sector, CoastType coast);
short			FindSector(short h, short v, CoastType *coast);
int 			neighborsector (UnitType unit, short start, CoastType s_coast, short finish, 
   								CoastType f_coast, short supp);
void			unitmapconsist();

#define sunit2string(unit) (unit == U_FLT?"F":"A")
#define lunit2string(unit) (unit == U_FLT?"Fleet":"Army")
#define sreg2string(reg) (map[reg].names[1])
#define lreg2string(reg) (map[reg].names[0])
#define scoast2string(cst) (scoast[cst])
#define lcoast2string(cst) (lcoast[cst])
#define country2string(coun) (countries[coun].name)
#define cntadj2string(coun) (countries[coun].adjective)

#define getsector(m,index) (m+(index))
#define issupp(r) ((r)->type & F_SUPP)
#define island(r) ((r)->type & F_LAND)
#define issea(r) ((r)->type & F_SEA)
#define iscoastal(r) (((r)->type & F_LAND) && ((r)->type & F_SEA))
#define isbicoastal(r) ((r)->coast1 != C_NONE)
#define isice(r) ((r)->type & F_ICE)
#define isisthmus(r) ((r)->type & F_ISTH)
#define isconvoyable(r) ((r)->type & F_CONV)
#define landneigh(r) ((r)->land)
#define seaneigh(r) ((r)->sea1)
#define coast1neigh(r) ((r)->sea1)
#define coast2neigh(r) ((r)->sea2)

#define startsector(m) ((m)+1)
#define issector(r) (sectindex(r) <= NUM_SECT)
#define nextsector(r) ((r)++)
#define sectindex(r) ((r)-map)

#define sname(r) ((r)->name)
#define ssname(r) ((r)->shortname)
#define scoast1(r) ((r)->coast1)
#define scoast2(r) ((r)->coast2)

#define sectunit(s) ((s)->unit)

char *string2sector(short *sector, char *s);
char *string2coast(CoastType *coast, char *s);
char *string2country(short *country, char *s);
CoastType char2coast(char coastdata);
char coast2char(CoastType coast);
short char2country(char c);

#endif