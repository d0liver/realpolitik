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
#include "Variants.h"
#include "FileFinder.h"
#include "Game.h"
#include "Global.h"
#include "History.h"
#include "MapDisplay.h"
#include "MenuUtil.h"
#include "Parse.h"
#include "Preferences.h"
#include "Resource.h"
#include "Textfile.h"
#include "Util.h"
#include <stdio.h>
#include <string.h>

#define MAX_VARS 32

short gCurrVariant;
short gNumVariants;
struct variant variantList[MAX_VARS+1];
struct variant *variantInfo;
char  gInstallPath[1024];

bool ReplaceSeparator(char *filename);
bool GetNextFilename(TextFile& fp, char* path, char *filename, char* header);

int InitVariants(void)
{
	// count and read in the variant data
	char path[1024];
	(void) GetInstallDirectory(path);
	strncat(path, kDataDirectory, 1023);
	strncpy(gInstallPath, path, 1023);
	
	FileFinder finder;
	if (!finder.Start(path))
		return 0;

	gNumVariants = 0; 
	char subfolder[256], found[256], filename[256];
	// go through each folder in data directory
	while (finder.Next(subfolder))
	{
		char subfolderpath[1024];
		FileFinder varFinder;
		char* space = 0;
		
		strcpy(subfolderpath, path);
		strcat(subfolderpath, subfolder);
		strcat(subfolderpath, kSeparator);
		// if not a directory
		if (!varFinder.Start(subfolderpath))
			continue;
	
		// go through each file in subfolder
		while (varFinder.Next(found))
		{
			// make sure it's a variant file
			if (!strstr(found, ".var"))
				continue;
	
			// open the file
			strcpy(filename, subfolderpath);
			strcat(filename, found);

			// read in the data
			FSSpec fsspec;
			TextFile file;
			char line[256];
			char header[30];
			char data[256];
			unsigned short flags;
			MakeFSSpec(fsspec, filename);
			if (!file.Open(fsspec, false))
			{	
				sprintf(header, "Can't open variant file %s", subfolderpath);
				ErrorString(header);
				continue;
			}
			
			strcpy(variantList[gNumVariants].directory, subfolderpath);

			file.ReadLine(line, 255);
			sscanf(line, "%s %hd", header, &variantList[gNumVariants].version);
			if (strcmp(header, "Version:") != 0)
				goto headerError;
			file.ReadLine(line, 255);
			if (strncmp(line, "Name:", 5) != 0)
				goto headerError;
			space = skipword(line);
			if (space != NULL)
			{
				strncpy(variantList[gNumVariants].name, space, 29);
			}

			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].map, "MapData:"))
				goto headerError;
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].countries, "Countries:"))
				goto headerError;
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].game, "Game:"))
				goto headerError;
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].bwmap, "BWMap:"))
				goto headerError;
			if (!strchr(variantList[gNumVariants].bwmap, '.'))
				strcat(variantList[gNumVariants].bwmap, kBitmapExtension);
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].colormap, "ColorMap:"))
				goto headerError;
			if (!strchr(variantList[gNumVariants].colormap, '.'))
				strcat(variantList[gNumVariants].colormap, kBitmapExtension);
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].polymap, "Regions:"))
				goto headerError;
			if (!GetNextFilename(file, subfolderpath, variantList[gNumVariants].info, "Info:"))
				goto headerError;
			// get build type
			file.ReadLine(line, 255);
			sscanf(line, "%s %s", header, data);
			if (strcmp(header, "Build:") != 0)
				goto headerError;
			uppercase(data);
			if (strcmp(data, "ABERRATION") == 0)
			{
				variantList[gNumVariants].flags = VF_ABERRATION;
			}
			else if (strcmp(data, "CHAOS") == 0)
			{
				variantList[gNumVariants].flags = VF_CHAOS;
			} 
			else
			{
				variantList[gNumVariants].flags = 0;
			}
			
			// get winning sectors
			file.ReadLine(line, 255);
			sscanf(line, "%s %hd", header, &flags);
			if (strcmp(header, "Centers:") != 0)
				goto headerError;
			variantList[gNumVariants].flags |= (((unsigned short)flags) << 8); 
			
			// get any other flags
			file.ReadLine(line, 255);
			sscanf(line, "%s %hd", header, &flags);
			if (strcmp(header, "Flags:") != 0)
				goto headerError;
			variantList[gNumVariants].flags |= flags; 

			// move standard variant to beginning
			if (strcmp(found, "Standard.var") == 0)
			{
				if (gNumVariants != 0)
				{
					struct variant tmpVariant = variantList[gNumVariants];
					// shift everybody up
					for (int i = gNumVariants; i > 0; i--)
					{
						variantList[i] = variantList[i-1];
					}
					variantList[0] = tmpVariant;
				}
			}
			// sort the rest
			else
			{	
				int index = 0;
				// if standard already found
				if (strcmp(variantList[0].name, "Standard") == 0)
					index = 1;
				for (; index < gNumVariants; ++index)
				{
					if (strcmp(variantList[index].name, variantList[gNumVariants].name) > 0)
						break;
				}
				if (index < gNumVariants)
				{
					struct variant tmpVariant = variantList[gNumVariants];
					// shift everybody up
					for (int i = gNumVariants; i > index; i--)
					{
						variantList[i] = variantList[i-1];
					}
					variantList[index] = tmpVariant;
				}

//				AddToVariantMenu(variantList[gNumVariants].name);
			}
		
			gNumVariants++;
			file.Close();
			continue;
			
headerError:			
			sprintf(header, "Can't load variant file %s: error at line %d", 
				subfolderpath, file.GetCurrentLine());
			ErrorString(header);
			file.Close();
		}
	}
	
	for (int i = 1; i < gNumVariants; ++i)
		AddToVariantMenu(variantList[i].name);


	gCurrVariant = -1;
	variantInfo = NULL;

	return gNumVariants;
}

bool SetVariant(short index)
{
	if (index == gCurrVariant)
		return false;
	if (index >= gNumVariants || index < 0)
		return false;
	
	gCurrVariant = index;
	variantInfo = &variantList[gCurrVariant];
	
	strcpy(prefs.mLastVariant, variantList[gCurrVariant].name);
	CheckVariantItem(gCurrVariant, TRUE);
	
	return true;
}

bool SetVariantByName(char* name)
{
	// find variant with right name
	for (short i = 0; i < gNumVariants; ++i)
	{
		if (strcmp(variantList[i].name, name) == 0)
		{
			return SetVariant(i);
		}
	}
	return false;
}

bool
NewVariant(short index)
{
	gErrNo = errNoError;
	if (index == gCurrVariant)
		return false;
	if (index >= gNumVariants)
		return false;	
	
	if (!CloseGame())
		return false;
		
	ShowWaitCursor();
	
	if (!ChangeVariant(index))
	{
 		ErrorAlert(gErrNo, gErrorString);
 		
 		// try with standard variant
		if (!ChangeVariant(0))
 		{
 			ShowArrowCursor();
 			return false;
 		}
	}

	if (!NewGame())
	{
  		ErrorAlert(gErrNo, gErrorString);
  		
		// try with standard variant before giving up
 		if (!ChangeVariant(0) || !NewGame())
 		{
 			ShowArrowCursor();
 			return false;
 		}
	}
		
	ShowArrowCursor();
 	return true;
		
}

bool ChangeVariant(short index)
{
	gErrNo = errNoError;
	if (index == gCurrVariant)
		return false;
	if (index >= gNumVariants)
		return false;	
	
	// clear history before clearing data files
	history.Clear();
	CleanMapDisplay(); 
	
//	printf("Free mem: %ld\n", FreeMem()); 
	
	CheckVariantItem(gCurrVariant, FALSE);
	(void) SetVariant(index);
	
 	if (!InitMapDisplay())
 		return false;
 		
	return true;
}

int
string2variant(char *variant)
{
	register short i;
	
	for (i = 0; i < gNumVariants; i++)
	{
		if (!strcmp(variant, variantList[i].name))
			return i;
	}
	
	return -1;
}

bool GetNextFilename(TextFile& file, char* path, char* filename, char* desiredHeader)
{
	char header[256];
	char buffer[256];
	char line[256];
	memset(buffer, 0, sizeof(buffer));
	file.ReadLine(line, 255);
	sscanf(line, "%s %s", header, buffer);
	if (strcmp(header, desiredHeader) != 0)
	{
		return false;
	}
	if (ReplaceSeparator(buffer))
	{
		strcpy(filename, gInstallPath);
	}
	else
	{
		strcpy(filename, path);
	}
	strcat(filename, buffer);
	
	return true;
}

bool
ReplaceSeparator(char *filename)
{
	if (filename == NULL)
		return false;
	bool result = false;
		
	while (*filename != '\0')
	{
		if (*filename == '/')
		{
			*filename = *kSeparator;
			result = true;
		}
		filename++;
	}
	return result;
}
		