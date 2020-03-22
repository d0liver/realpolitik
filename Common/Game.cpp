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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MacTypes.h"
#include "Game.h"
#include "History.h"
#include "Unit.h"
#include "MapData.h"
#include "MenuUtil.h"
#include "MapDisplay.h"
#include "OrdersDisplay.h"
#include "OrderPalette.h"
#include "OldGame.h"
#include "StatusDisplay.h"
#include "TextFile.h"
#include "Variants.h"
#include "Util.h"
#include "Strings.h"
#include "ReadOrders.h"
#include "MapContent.h"
#include "WindowUtil.h"
#include "Resource.h"
#include "EditMap.h"
#include "Global.h"

/* globals */
bool gGameRunning = false;
bool gGameChanged = false;
bool gSaveDislodge = false;
bool gIgnoreUnits = false;
bool gStrictNMR = true;
bool gResolve = true;
bool gCommit = false;
FSSpec gGameFileName;

char gamename[30] = "";
char variant[30] = "";
short season = S_SPRING;
short year = 1901;
short adjustments = 0;

short winner = 0;


/* local prototypes */
bool  InitGame(void);
bool  ReadGame(const FSSpec& fileName, bool newMapWindow = true);
bool  WriteGame(const FSSpec& fileName);
bool	 ScanData(TextFile& infile);

bool  findwinner();


/* NewGame() - set up windows and stuff for a new game 
 *             returns true if all went well, false otherwise
 */
bool
NewGame()
{
	char	maptitle[256];
	
	// need to close old game first
	if (!CloseGame())
		return false;

	ShowWaitCursor();
	
	if (!InitGame())
	{
		ShowArrowCursor();
		return false;
	}

	orderPalette.Activate( mapWindow, true );
	orderPalette.Reset();
	
	if (!NewMapWindow())
	{
		ShowArrowCursor();
		return false;
	}
	if (strlen(gamename) > 0)
	{
		SetMapTitle(gamename);
	}
	else
	{
		GetIndCString(maptitle, kSpecialStringsID, kDefaultMapTitleID);
		SetMapTitle(maptitle);
	}
	DrawNewMap();
		
	NextOrders();
	OutputStatus();

	ClearFSSpec(gGameFileName);

	gGameRunning = true;
	gSaveDislodge = false;
	gGameChanged = false;
	
	gResolve = true;
	gCommit = false;
	
	ShowMapWindow();

	CheckIgnoreUnitsItem(gIgnoreUnits); 
		
	ShowArrowCursor();

	return true;

} /* NewGame */


/* 
 * OpenGame() - open game file, set up windows and stuff 
 *              returns true if everything worked, false otherwise
 */
bool
OpenGame()
{
	FSSpec filename;
	char* titlename = {0};
	
	if (!GetFile(filename, titlename, gameFilter))
		return false;
		
	/* need to close old game */
	if (!CloseGame())
		return false;
		
	return OpenFinderGame(filename);
	
} /* OpenGame */

/* 
 * OpenFinderGame() - open game file from finder, set up windows and stuff 
 *              returns true if everything worked, false otherwise
 */
bool
OpenFinderGame(const FSSpec& fName)
{	
	ShowWaitCursor();
	
	CopyFSSpec(gGameFileName, fName);
	
	gIgnoreUnits = false;
	gStrictNMR = true;
	CheckIgnoreUnitsItem(gIgnoreUnits);
	gResolve = true;
	gCommit = false;

	if (!ReadGame(fName))
	{
		ShowArrowCursor();
		return false;
	}
		
	gGameRunning = true;
	gGameChanged = false;
	
//	ShowMapWindow();

	ShowArrowCursor();

	return true;

} /* OpenFinderGame */

/*
 * CloseGame() - check for save, then close up game 
 *               returns true if really quit, false otherwise
 */
bool
CloseGame()
{
	short result;
	register short i;
	
	if (!gGameRunning)		/* nothing to close */
	    return true;
	
	if (gGameChanged)
	{
		result = CheckSave();
	
		if (result == kSaveCancel) 
			return false;		
		else if (result == kSaveYes) 
			if (!SaveGame())
				return false;
	}
	
	if (EditModeOn())
	{
		ExitEditMode();
		CheckEditMapItem(EditModeOn());
	}
		
/*	CloseMapWindow(); */
	HideOrdersWindow(); 
	HideStatusWindow(); 
	
	gGameRunning = false;
	gGameChanged = false;
	
	/* free up pointers */
	for (i = 0; i <= NUM_COUN; i++)
	{
		if (units != NULL)
			cleanunits(&units[i]);
		if (supplies != NULL)
			cleansupplies(&supplies[i]);
		if (orders != NULL)
			cleanorders(orders[i]);
		countries[i].builds = 0;
	}
    
    dslglist.Clean();
    dislodges = 0;
    winner = 0;
    
	ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();
     
	return true;
	
} /* CloseGame */


/*
 * InitGame() - load in resource info for game
 *              returns true if successful, false otherwise
 */
bool
InitGame()
{
	bool result;
	FSSpec filename;
	TextFile infile;
	MakeFSSpec(filename, variantInfo->game);
	if (!infile.Open(filename, false))
		return false;
	
	gamename[0] = '\0';

	// get the data
	result = ScanData(infile);

	infile.Close();

	return result;
}

bool
ScanData(TextFile& infile)
{
	char   line[256], string[256];
	short  version;
	register short  count, i, j;
	short  var;

	if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &version) != 1)
	{	
		gErrNo = errBadGameData;
		strcpy(gErrorString, "Invalid version");
		return false;
	}
	if (version != 1)
		return false;

	if (!infile.ReadLine(gamename, 255))
	{	
		gErrNo = errBadGameData;
		strcpy(gErrorString, "Invalid game name");
		return false;
	}
	if (gamename[strlen(gamename)-1] == '\n')
		gamename[strlen(gamename)-1] = '\0';

	if (!infile.ReadLine(variant, 255))
	{	
		gErrNo = errBadGameData;
		strcpy(gErrorString, "Invalid variant name");
		return false;
	}
	if (variant[strlen(variant)-1] == '\n')
		variant[strlen(variant)-1] = '\0';

	var = string2variant(variant);
	if (var < 0)
	{
		gErrNo = errBadGameData;
		strcpy(gErrorString, "Invalid variant name");
		return false;
	}
	
	if (!ChangeVariant(var) && gErrNo != errNoError)
		return false;

	if (!infile.ReadLine(line, 255) || sscanf(line, "%s %hd", string, &year) != 2)
	{	
		gErrNo = errBadGameData;
		strcpy(gErrorString, "Invalid season");
		return false;
	}
	season = string2season(string);
	
	if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &adjustments) != 1)
	{	
		gErrNo = errBadGameData;
		sprintf(gErrorString, "Invalid adjustments at line %d", infile.GetCurrentLine());
		return false;
	}
	if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &count) != 1)
	{	
		gErrNo = errBadGameData;
		sprintf(gErrorString, "Invalid country count");
		return false;
	}
	if (count != NUM_COUN)
	{
		gErrNo = errBadGameData;
		sprintf(gErrorString, "Invalid country count");
		return false;
	}
	
	for (i = 1; i <= NUM_COUN; i++)
	{
		if (!infile.ReadLine(line, 255))
		{
			sprintf(gErrorString, "Invalid builds at line %d", infile.GetCurrentLine());
			gErrNo = errBadGameData;
			return false;
		}

		countries[i].builds = atoi(line);
		
		if (!infile.ReadLine(line, 255) || strlen(line) < 2)
			supplies[i] = NULL;
		else if ((supplies[i] = string2slist(line, i)) == NULL)
		{
			sprintf(gErrorString, "Invalid supply list at line %d", infile.GetCurrentLine());
			gErrNo = errBadGameData;
			return false;
		}

		if (!infile.ReadLine(line, 255) || strlen(line) < 2)
			units[i] = NULL;
		else if ((units[i] = string2ulist(line, i)) == NULL)
		{
			sprintf(gErrorString, "Invalid unit list at line %d", infile.GetCurrentLine());
			gErrNo = errBadGameData;
			return false;
		}

		if (slistsize(supplies[i]) >= win_supply)
			winner = i;
	}
	
	// dislodges 
	if (!infile.ReadLine(line, 255) || sscanf(line, "%hd", &dislodges) != 1)
	{	
		sprintf(gErrorString, "Invalid dislodge count at line %d", infile.GetCurrentLine());
		gErrNo = errBadGameData;
		return false;
	}
	
	if (dislodges > 0 
		&& (season == S_SUMMER || season == S_AUTUMN)
		)
		gSaveDislodge = true;
	else
		gSaveDislodge = false;
		
	// read in dislodges
	for (i = 0; i < dislodges; i++)
	{
		if (!infile.ReadLine(line, 255))
		{
			sprintf(gErrorString, "Invalid dislodge string at line %d", infile.GetCurrentLine());
			gErrNo = errBadGameData;
			return false;
		}
		Dislodge* dislodge = new Dislodge();
		if (string2dislodge(dislodge, line))
		{
			(void) dslglist.Add( dislodge );
		}
		else
		{
			delete dislodge;
			sprintf(gErrorString, "Invalid dislodge string at line %d", infile.GetCurrentLine());
			gErrNo = errBadGameData;
			return false;
		}
	}		
	
	// history
	if (!history.Read( infile ))
	{
		sprintf(gErrorString, "Invalid history at line %d", infile.GetCurrentLine());
		gErrNo = errBadGameData;
		return false;
	}
	// add current state to end of history
	history.AppendCurrentState();

	// get unowned supply centers
	for (j = 1; j <= NUM_SECT; j++)
	{
		if (issupp(map+j))
		{
			for (i = 1; i <= NUM_COUN; i++)
			{
				if (inslist(supplies[i], j))
					break;
			}
			if (i > NUM_COUN)
				addsupply(&supplies[0], j, 0);
		}
	}

	return true;
}

/* 
 * ReadGame() - read in game from file 
 *              returns true if successful, false otherwise
 */
bool
ReadGame(const FSSpec& gameFileName, bool newMapWindow)
{
	bool result;
	TextFile infile;
	char basename[256];
	if (!infile.Open(gameFileName, false))
	{
		gErrNo = errBadGameData;
		GetBasename(basename, gameFileName);
		sprintf(gErrorString, "File %s not found", basename);
		return false;
	}
	
	gamename[0] = '\0';

	// get the data
	result = ScanOldData(gameFileName);
	if (!result)
		result = ScanData(infile);

	if (!result)
	{
		gErrNo = errBadGameData;
		infile.Close();	
		return false;
	}

	orderPalette.Reset();
	orderPalette.Activate( mapWindow, true );

	NextOrders();

	if (newMapWindow && !NewMapWindow())
		return false;	
	if (strlen(gamename) > 0)
	{
		SetMapTitle(gamename);
	}
	else 
	{
		GetBasename(basename, gGameFileName);
		SetMapTitle(basename);
	}
	
	if (!LoadGameOrders(infile))
		DrawNewMap();
	
	infile.Close();	

	return true;
}


/* 
 * SaveGame() - brings up Save dialog if necessary, and writes game to 
 *              that file.
 *              returns true if okay, false otherwise
 */
bool
SaveGame()
{
	if (!gGameRunning || !gGameChanged)
		return false;
		
	// if we have no filename to save to
	if (EmptyFSSpec(gGameFileName))
		if (SaveGameAs())
		{
			gGameChanged = false;
			return true;
		}
		else return false;
	else
		if (WriteGame(gGameFileName))
		{
			gGameChanged = false;
			return true;
		}
		else return false;
}


/* 
 * SaveGameAs() - open save dialog, get filename, and write to file
 *                returns true if successful, false otherwise
 */
bool
SaveGameAs()
{
	char prompt[256];
	char defaultName[256];
	FSSpec filename;
	
	GetIndCString(prompt, kSpecialStringsID, kSaveGameStringID);
	// if we haven't saved this game yet
	if (EmptyFSSpec(gGameFileName))
	{
		// and if this game has no name
		if (strlen(gamename) == 0)
		{
			// set it to be the default
			GetIndCString(defaultName, kSpecialStringsID, kDefaultGameNameID);
		}
		// otherwise set the default to be the game name
		else
			strcpy(defaultName, gamename);
	}
	// else set the default to be the game file name
	else
		GetBasename(defaultName, gGameFileName);

	// get the save file name
	if (!PutFile(filename, prompt, defaultName, gameFilter))
		return false;
		
	// if game name is same as variant name, change to file name
//	if (strcmp(gamename, variant) == 0)
	{
		GetBasename(gamename, filename);
		// strip any .dpy off the end	
		short length = strlen(gamename);
		if ( length >= 4 && gamename[length-4] == '.' 
			&& gamename[length-3] == 'd' 
			&& gamename[length-2] == 'p' 
			&& gamename[length-1] == 'y' )
		{
			gamename[length-4] = '\0';	
		}
	}
		
	CopyFSSpec(gGameFileName, filename);
	if (WriteGame(filename))
		gGameChanged = false;
	else
		return false;
		
	if (strlen(gamename) == 0)
	{
		SetMapTitle(defaultName);
	}
	else
	{
		SetMapTitle(gamename);
	}
	
	return true;	
}


/*
 * WriteGame() - write game to file
 *               returns true if successful, false otherwise 
 */
bool
WriteGame(const FSSpec& gameFileName)
{
	char string[256];
	int i;
	bool result = true;
	int version = 1;

	TextFile outfile;
	if (!outfile.Open(gameFileName, true))
		return false;

	if (!sprintf(string, "%d", version) || !outfile.WriteLine(string))
	{	
		gErrNo = errFile;
		return false;
	}

	if (!outfile.WriteLine(gamename))
	{	
		gErrNo = errFile;
		return false;
	}

	if (!outfile.WriteLine(variant))
	{	
		gErrNo = errFile;
		return false;
	}
	
	if (!history.AtEnd())
	{
		if (!history.WriteLastState( outfile ))
		{
			gErrNo = errFile;
			return false;
		}
	}
	else
	{
		GetIndCString(string, kSeasonStringsID, season);
		if (!sprintf(string, "%s %d", string, year) || !outfile.WriteLine(string))
		{	
			gErrNo = errFile;
			return false;
		}
	
		
		if (!sprintf(string, "%d", adjustments) || !outfile.WriteLine(string))
		{	
			gErrNo = errFile;
			return false;
		}
		
		if (!sprintf(string, "%d", NUM_COUN) || !outfile.WriteLine(string))
		{	
			gErrNo = errFile;
			return false;
		}
		
		for (i = 1; i <= NUM_COUN; i++)
		{
			if (!sprintf(string, "%d", countries[i].builds) || !outfile.WriteLine(string))
			{	
				gErrNo = errFile;
				return false;
			}
	
			memset(string, 0, sizeof(string));
			slist2string(string, supplies[i], true);
			if (!outfile.WriteLine(string))
			{
				gErrNo = errFile;
				return false;
			}
			
			memset(string, 0, sizeof(string));
			ulist2string(string, units[i], true);
			if (!outfile.WriteLine(string))
			{
				gErrNo = errFile;
				return false;
			}		
		}
		
		// dislodges 
		if (!sprintf(string, "%d", dislodges) || !outfile.WriteLine(string))
		{	
			gErrNo = errFile;
			return false;
		}
		
		for (Dislodge* current = dslglist.Start(); current != dslglist.End(); current = dslglist.Next(current) )
		{
			memset(string, 0, sizeof(string));
			dislodge2string(string, current);
			if (!outfile.WriteLine(string))
			{
				gErrNo = errFile;
				return false;
			}		
		}		
	}
		
	// history
	if (!history.Write(outfile))
	{
		gErrNo = errFile;
		return false;
	}		

	// only write out game orders at end
	if (history.AtEnd())
	{
		result = WriteGameOrders(outfile);
	}
	else
	{
		result = true;
	}

	outfile.Close();
	
	return result; 
}

/*
 * RevertGame() - revert game to saved copy
 *                returns true if successful, false otherwise 
 */
bool
RevertGame()
{	
	register short i;
	char maptitle[256];
	
	if (!gGameRunning || !gGameChanged)
		return false;
		
	gGameRunning = false;
	gGameChanged = false;
	
	/* free up pointers */
	for (i = 0; i <= NUM_COUN; i++)
	{
		if (units != NULL)
			cleanunits(&units[i]);
		if (supplies != NULL)
			cleansupplies(&supplies[i]);
		if (orders != NULL)
			cleanorders(orders[i]);
		countries[i].builds = 0;
	}
    
    dslglist.Clean();
    dislodges = 0;
    winner = 0;
    
    ClearClicks();
    ClearOrderSelections();
    ClearSelectedSector();
    ClearLastOrder();
    
	gResolve = true;
	gCommit = false;

	if (EmptyFSSpec(gGameFileName))
	{
		if (!InitGame())
			return false;
		orderPalette.Reset();			
		orderPalette.Activate( mapWindow, true );
		if (strlen(gamename) > 0)
		{
			SetMapTitle(gamename);
		}
		else
		{
			GetIndCString(maptitle, kSpecialStringsID, kDefaultMapTitleID);
			SetMapTitle(maptitle);
		}
		NextOrders();
		ClearFSSpec(gGameFileName);
		gSaveDislodge = false;
		DrawNewMap();
		UpdateMapDisplay(NULL);
		OutputStatus();
	
	}
	// read without reinitializing map window
	else if (!ReadGame(gGameFileName, false))
		return false;
		
	gGameRunning = true;
	gGameChanged = false;
		
	CheckIgnoreUnitsItem(gIgnoreUnits); 
		
	return true;
}


/*
 * QuitGame() - close and quit the game
 *              returns true if successful, false otherwise
 */
bool
QuitGame()
{

	if (!CloseGame())
		if (gErrNo != errNoError)
			return true;
		else
			return false;
	return true;

}


short
string2season(char *gPointer)
{
	short i;
	char string[256];
	
	for (i = 1; i < S_MAX; i++)
	{
		GetIndCString(string, kSeasonStringsID, i);
		if (!strncmp(gPointer, string, strlen(string)))
			return i;
	}
	
	return S_NONE;
}
		
void
nextseason()
{
	switch (season)
	{
	case S_SPRING:
		if (dislodges)
			season = S_SUMMER;
		else
			season = S_FALL;
		break;
		
	case S_SUMMER:
		season = S_FALL;
		break;
		
	case S_FALL:
		if (dislodges)
			season = S_AUTUMN;
		else if (adjustments)
			season = S_WINTER;
		else
		{
			season = S_SPRING;
			year++;
		}
		break;
		
	case S_AUTUMN:
		if (adjustments)
			season = S_WINTER;
		else
		{
			season = S_SPRING;
			year++;
		}
		break;
		
	case S_WINTER:
		season = S_SPRING;
		year++;
		break;
	}
	
	(void) findwinner();
}

bool
findwinner()
{
	register short i;
	
	for (i = 1; i <= NUM_COUN; i++)
	{
     	if (slistsize(supplies[i]) >= win_supply)
     	{
     		winner = i;
     		return true;
     	}
   	}
   	
   	return false;
}

