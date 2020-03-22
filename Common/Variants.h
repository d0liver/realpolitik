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
struct variant 
{
	short version;
	char directory[128];
	char name[128];
	char map[128];
	char countries[128];
	char game[128];
	char bwmap[128];
	char colormap[128];
	char polymap[128];
	char info[128];
	unsigned short flags;
};

#define VF_ABERRATION 	0x1
#define VF_CHAOS  		0x2

extern struct variant *variantInfo;

int InitVariants(void);
bool SetVariant(short index);
bool SetVariantByName( char* name );
bool ChangeVariant(short index);
bool NewVariant(short index);
int string2variant(char *variant);
