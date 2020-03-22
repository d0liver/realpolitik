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
#include "MapDisplay.h"
#include <string.h>
#include "Dislodge.h"
#include "Global.h"
#include "Image.h"
#include "MapContent.h"
#include "MapData.h"
#include "Offscreen.h"
#include "Order.h"
#include "PieceIcon.h"
#include "Preferences.h"
#include "Rgn.h"
#include "Strings.h"
#include "Util.h"
#include "Variants.h"

#define BWMAP 0
#define COLORMAP 0
#define POLYS 0

extern OffscreenPtr mapPort;
extern ImagePtr mapPict, cMapPict;

extern bool gSaveDislodge;

PieceIcon* unitIcons[5*MAX_COUN] = {0};
static short num_icons = 0;
static short selected_sector = 0;
static bool dislodged_unit = 0;

void DrawMove(Order o, bool bw, bool gs);
void DrawSupp(Order o, bool bw, bool gs);
void DrawConvs(bool bw, bool gs);
void DrawBuild(Order o, bool bw, bool gs);
void DrawRemove(Order o, bool bw, bool gs);
void DrawOutline(bool BW, UnitType type, short sector, CoastType coast, short country);
void DrawX(bool BW, UnitType type, short sector, CoastType coast, short country, short offset);
void DrawDislodge(bool BW, UnitType type, short sector, CoastType coast, short country);


bool LoadCountryIcons()
{
	int i, country;
	char iconPath[128];
	// see if there are icons in the variant folder
	strcpy(iconPath, variantInfo->directory);
	strcat(iconPath, kIconPath);
	if (!PieceIcon::InitIconResources(iconPath))
	{ 	
		// if not, use the defaults
		(void) GetInstallDirectory(iconPath);
		strcat(iconPath, kIconPath);
		if (!PieceIcon::InitIconResources(iconPath))
		{
			gErrNo = errBadCountryData;
			strcpy(gErrorString, "Couldn't find icon directory");
			return false;
		}
	}
	
	// two for each country 
	num_icons = NUM_COUN*5;
	i = 0;

	char name[2] = {0,0};
	char iconName[256];
	char fullName[256];
	for (country = 1; country <= NUM_COUN; country++)
	{
		// get initial
		name[0] = countries[country].abbreviation;
		unitIcons[i] = new PieceIcon(name);
		if (unitIcons[i] == NULL || !unitIcons[i]->IsInitialized())
		{
			// don't require initial icons
//			gErrNo = errBadCountryData;
//			strcpy(gErrorString, "Couldn't find letter icon for ");
//			strcat(gErrorString, countries[country].name);
//			return false;
		}
		i++;

		// get color army
		GetIndCString(iconName, kColorStringsID, countries[country].colorID);
		strcpy(fullName, iconName);
		strcat(fullName, "Army");
		unitIcons[i] = new PieceIcon(fullName);
		if (unitIcons[i] == NULL || !unitIcons[i]->IsInitialized())
		{
			gErrNo = errBadCountryData;
			strcpy(gErrorString, "Couldn't find color army icon for ");
			strcat(gErrorString, countries[country].name);
			return false;
		}
		i++;		

		// get color fleet
		GetIndCString(iconName, kColorStringsID, countries[country].colorID);
		strcpy(fullName, iconName);
		strcat(fullName, "Fleet");
		unitIcons[i] = new PieceIcon(fullName);
		if (unitIcons[i] == NULL || !unitIcons[i]->IsInitialized())
		{
			gErrNo = errBadCountryData;
			strcpy(gErrorString, "Couldn't find color fleet icon for ");
			strcat(gErrorString, countries[country].name);
			return false;
		}
		i++;		

		// get pattern army
		GetIndCString(iconName, kPatternStringsID, countries[country].patID);
		strcpy(fullName, iconName);
		strcat(fullName, "Army");
		unitIcons[i] = new PieceIcon(fullName);
		if (unitIcons[i] == NULL || !unitIcons[i]->IsInitialized())
		{
//			gErrNo = errBadCountryData;
//			strcpy(gErrorString, "Couldn't find pattern army icon for ");
//			strcat(gErrorString, countries[country].name);
//			return false;
		}
		i++;	
			
		// get pattern fleet
		GetIndCString(iconName, kPatternStringsID, countries[country].patID);
		strcpy(fullName, iconName);
		strcat(fullName, "Fleet");
		unitIcons[i] = new PieceIcon(fullName);
		if (unitIcons[i] == NULL || !unitIcons[i]->IsInitialized())
		{
//			gErrNo = errBadCountryData;
//			strcpy(gErrorString, "Couldn't find pattern fleet icon for ");
//			strcat(gErrorString, countries[country].name);
//			return false;
		}
		i++;		
	}

	return true;
}

void CleanIcons()
{
	register short i;

	for (i = 0; i < num_icons; i++)
	{
		if (unitIcons[i] != NULL)
		{
			delete unitIcons[i];
		}
		unitIcons[i] = NULL;
	}
	num_icons = 0;
	
	PieceIcon::ReleaseIconResources();
}


void
DrawNewMap(void)
{
	short 		depth = 1;
	bool monochrome = mapPort->IsMonochrome();
	bool grayScale = mapPort->IsGrayScale();
	Rect		mapRect;

	mapPort->SetAsCurrent();

	PenNormal();
	mapPort->Lock();
	
	// clear the background
//	PatBlt(CurrentDC(), mapRect.left, mapRect.top, mapRect.right, mapRect.bottom, WHITENESS);		

	if (monochrome)
	{
		mapPict->GetRect( mapRect );
		mapPict->Draw( mapRect );
	}
	else
	{
		cMapPict->GetRect( mapRect );
		cMapPict->Draw( mapRect );
	}

	if (prefs.mShowSupplies)
		DrawSupplies(monochrome || grayScale);
		
	if (prefs.mShowNames)
		DrawNames(false);
		
	if (prefs.mShowOrders)
		DrawOrders(monochrome, grayScale);
		
	if (prefs.mShowUnits && dislodges && gSaveDislodge)
		DrawDislodges(monochrome, grayScale);
		
	if (prefs.mShowUnits)
		DrawUnits(monochrome);
				
	if (prefs.mShowOrders && !dislodges)
		DrawAdjusts(monochrome, grayScale); 

	mapPort->Unlock();
	mapPort->RestorePrevious();

}



void
DrawUnits(bool BW)
{
	register short i;
	register Unit currunit;
	
	for (i = 1; i <= NUM_COUN; i++)
	{
		for (currunit = startunit(units[i]); isunit(currunit); nextunit(currunit))
		{
			DrawUnit(BW, utype(currunit), usector(currunit), ucoast(currunit), i);
		}
	}
	
	
}

void
DrawDislodges(bool BW, bool gs)
{
	Order op;
	Dislodge* current;
		
	for (current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next( current ))
	{
		DrawDislodge(BW, current->Unit(), current->Sector(), current->Coast(), current->Owner());
		if (dislodges)
		{
			for (op = startorder(orders[current->Owner()]);
				 isorder(op); nextorder(op))
			{
				if (ostart(op) == current->Sector() && otype(op) == O_DISBAND)
						DrawX(BW | gs, ounit(op), ostart(op), oscoast(op), ocountry(op), 4);
			}
		}
	}
	
}

/*
 * redraw only this unit
 */
void 
RedrawUnit(bool BW, UnitType type, short sector, CoastType coast, short country, 
			bool dislodge, bool remove)
{
	if (!prefs.mShowUnits)
		return;

	mapPort->SetAsCurrent();
	mapPort->Lock();

	if (!dislodge)
	{
		DrawUnit(BW, type, sector, coast, country);
		if (remove)
			DrawX(BW, type, sector, coast, country, 0);
	}
	else
	{
		DrawDislodge(BW, type, sector, coast, country);
		if (remove)
			DrawX(BW, type, sector, coast, country, 4);
		if (map[sector].unit != NULL)
		{
			DrawUnit(BW, utype(map[sector].unit), sector, ucoast(map[sector].unit),
					uowner(map[sector].unit));
		}
	}
	
	mapPort->Unlock();
	mapPort->RestorePrevious();
}

void GetUnitRect(Rect *theRect, UnitType type, int sector, CoastType coast)
{
	Rect iconRect = { -16, -16, 16, 16 };
	int offh, offv;
	
	if (type == U_ARM)
	{
		offh = map[sector].land_loc.h;
		offv = map[sector].land_loc.v;
	}
	else if (type == U_FLT)
	{
		if (coast == C_NONE || !map[sector].coast1)
		{
			offh = map[sector].land_loc.h;
			offv = map[sector].land_loc.v;
		}
		else if (coast == map[sector].coast1)
		{
			offh = map[sector].coast1_loc.h;
			offv = map[sector].coast1_loc.v;
		}
		else if (coast == map[sector].coast2)
		{
			offh = map[sector].coast2_loc.h;
			offv = map[sector].coast2_loc.v;
		}
	}
	
	OffsetRect(&iconRect, offh, offv);
	
	*theRect = iconRect;
}




void
DrawUnit(bool BW, UnitType type, short sector, CoastType coast, short country)
{
	PieceIcon *coloricon, *initialicon, *patternicon;
	Rect iconRect;
	
	country--;
	if (type == U_ARM)
	{
		coloricon = unitIcons[country*5 + 1];
		patternicon = unitIcons[country*5 + 3];
	}
	else if (type == U_FLT)
	{
		coloricon = unitIcons[country*5 + 2];
		patternicon = unitIcons[country*5 + 4];
	}

	initialicon = unitIcons[country*5];

	GetUnitRect(&iconRect, type, sector, coast);
	
	if (sector == selected_sector && !dislodged_unit)
	{
		if (BW)
			patternicon->DrawGreyed(iconRect, BW);
		else
			coloricon->DrawGreyed(iconRect, BW);
		if (initialicon)
			initialicon->DrawGreyed(iconRect, BW);
	}
	else
	{
		if (BW)
			patternicon->Draw(iconRect, BW);
		else
			coloricon->Draw(iconRect, BW);
		if (initialicon)
			initialicon->Draw(iconRect, BW);
	}
}


void
DrawDislodge(bool BW, UnitType type, short sector, CoastType coast, short country)

{
	PieceIcon *coloricon, *initialicon, *patternicon;
	Rect iconRect = { -12, -12, 20, 20 };
	
	country--;
	if (type == U_ARM)
	{
		coloricon = unitIcons[country*5 + 1];
		patternicon = unitIcons[country*5 + 3];
	}
	else if (type == U_FLT)
	{
		coloricon = unitIcons[country*5 + 2];
		patternicon = unitIcons[country*5 + 4];
	}

	initialicon = unitIcons[country*5];

	GetUnitRect(&iconRect, type, sector, coast);
	OffsetRect(&iconRect, 4, 4);
	
	if (sector == selected_sector && !dislodged_unit)
	{
		if (BW)
			patternicon->DrawGreyed(iconRect, BW);
		else
			coloricon->DrawGreyed(iconRect, BW);
		if (initialicon)
			initialicon->DrawGreyed(iconRect, BW);
	}
	else
	{
		if (BW)
			patternicon->Draw(iconRect, BW);
		else
			coloricon->Draw(iconRect, BW);
		if (initialicon)
			initialicon->Draw(iconRect, BW);
	}

}

void
DrawSupplies(bool bw)
{
	Pattern cntPattern;

	Supply currSupply;
	register short i;
	RGBColor color;

	for (i = 1; i <= NUM_COUN; i++)
	{
		GetIndPattern(&cntPattern, 128, countries[i].patID); 

		PenPat(&cntPattern);

		if (!bw)
		{
			SetCountryRGBColor( countries[i].colorID );
		}

		for (currSupply = startsupply(supplies[i]);
			 	issupply(currSupply); nextsupply(currSupply))
		{
			map[ssector(currSupply)].rgn->Draw( cntPattern );
		}

	}

	if (!bw)
	{
		// Back in Black 
		color.red = 0;
		color.green = 0;
		color.blue = 0;

		RGBForeColor(&color);
	}	

	PenNormal();
}


void
DrawNames(bool bw)
{
	char string[256];
	short fontNum, sector;
	Rect iconRect;
	RGBColor greyColor = {49344, 49344, 49344}, whiteColor = {0xFFFF, 0xFFFF, 0xFFFF},
		blackColor = {0x0, 0x0, 0x0};
	
	short offh, offv;
	
	strcpy(string, "Monaco");
	GetFNum(CtoPstr(string), &fontNum);
	TextFont(fontNum);
	TextSize(9);

	PenNormal();

	RGBForeColor(&blackColor);
	RGBBackColor(&whiteColor);
	if (!bw) RGBBackColor(&greyColor);
	for (sector = 1; sector <= NUM_SECT; sector++)
	{
		strcpy(string, sreg2string(sector));
		if (strlen(string) == 2)
		{
			SetRect(&iconRect, -7, -6, 8, 6);
		}
		else if (strlen(string) == 4)
		{
			SetRect(&iconRect, -13, -6, 14, 6);
		}
		else 
		{
			SetRect(&iconRect, -10, -6, 11, 6);
		}
		offh = map[sector].name_loc.h;
		offv = map[sector].name_loc.v;

		OffsetRect(&iconRect, offh, offv);
		EraseRect(&iconRect);
		FrameRect(&iconRect);
		MoveTo(iconRect.left+2, iconRect.bottom-3);
		DrawString(CtoPstr(string));

#if DOCOASTS
		if (map[sector].coast1)
		{
		SetRect(&iconRect, -9, -4, 12, 8);
			OffsetRect(&iconRect, map[sector].coast1_loc.h, map[sector].coast1_loc.v);
			EraseRect(&iconRect);
			FrameRect(&iconRect);
			strcpy(string, scoast2string(map[sector].coast1));
			CtoPstr(string);			
			MoveTo(map[sector].coast1_loc.h-7, map[sector].coast1_loc.v+5);
			DrawString(string);
		SetRect(&iconRect, -9, -4, 12, 8);
			OffsetRect(&iconRect, map[sector].coast2_loc.h, map[sector].coast2_loc.v);
			EraseRect(&iconRect);
			FrameRect(&iconRect);
			strcpy(string, scoast2string(map[sector].coast2));
			CtoPstr(string);			
			MoveTo(map[sector].coast2_loc.h-7, map[sector].coast2_loc.v+5);
			DrawString(string);
		} 
#endif
	}
	if (!bw) RGBBackColor(&whiteColor);

}

	
void
SetSelectedSector(short s, bool d)
{
	selected_sector = s;
	dislodged_unit = d;
}

void GetSelectedSector(short *s, bool *d)
{
	*s = selected_sector;
	*d = dislodged_unit;
}

void ClearSelectedSector()
{
	selected_sector = 0;
	dislodged_unit = 0;
}



void ResetSelectedUnit()
{
	Rect cliprect;
	short sector;
	bool dislodge;
	short c;
	Order oldorder;
	
	if (selected_sector != 0)
	{
		ClearClicks();
		sector = selected_sector;
		dislodge = dislodged_unit;
		ClearSelectedSector();
		for (c = 1; c <= NUM_COUN; c++)
		{
			if ((oldorder = findorder(orders[c], U_NONE, sector)) != NULL)
			{
				if ((otype(oldorder) == O_REMOVE || otype(oldorder) == O_DEF_REM
					|| otype(oldorder) == O_DISBAND) && !obadsyntax(oldorder))
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 		oscoast(oldorder), ocountry(oldorder), dislodge, true);
				else
					RedrawUnit(mapPort->IsMonochrome(), ounit(oldorder), sector,
				 		oscoast(oldorder), ocountry(oldorder), dislodge, false);
				GetUnitRect(&cliprect, ounit(oldorder), sector, oscoast(oldorder));
				break;
			}
		}			
		if (oldorder == NULL && map[sector].unit != NULL)
		{
			RedrawUnit(mapPort->IsMonochrome(), utype(map[sector].unit), 
				sector, ucoast(map[sector].unit), 
				uowner(map[sector].unit), dislodge, false);
			GetUnitRect(&cliprect, utype(map[sector].unit), sector, ucoast(map[sector].unit));
		}

		UpdateMapDisplay(&cliprect);
	}
}

