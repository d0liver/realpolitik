//===============================================================================
// @ WindowUtil.cpp
// ------------------------------------------------------------------------------
// Useful routines for manipulating windows
//
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
//
// Change history:
//
// 99-Dec-22	JMV	First version
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Includes -------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "WindowUtil.h"
#include <string.h>
#include <stdio.h>
#include "Global.h"
#include "Resource.h"
#include "MapDisplay.h"
#include "Util.h"
#include "Game.h"

//-------------------------------------------------------------------------------
//-- Static Variables -----------------------------------------------------------
//-------------------------------------------------------------------------------

static HWND mLoadingDialog = NULL;
extern HINSTANCE gInstance;

/* local defines */
#define kGameOverOK 1
#define kGameOverUserItem 3

//-------------------------------------------------------------------------------
//-- Functions ------------------------------------------------------------------
//-------------------------------------------------------------------------------
BOOL CALLBACK testProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam); 

void
DoAboutBox()
{
	static char messageString[] = 
"\tRealpolitik version 1.6.6.\n\n\
\tCopyright 2001-2002,\n\
The Realpolitik Development Team\n\n\
For support, join:\n\
RPForum@yahoogroups.com\n\
http://realpolitik.sourceforge.net\n\n\
Realpolitik comes with ABSOLUTELY NO\n\
WARRANTY.  This is free software, and\n\
you are welcome to redistribute it under\n\
certain conditions. See license.txt for details.";

	MessageBox( mapWindow->GetNative(), messageString, "About...", MB_OK );
/*CONVERT
	DialogPtr theDialog;
	Boolean done = 0;
	EventRecord  	event;
	
	if ((theDialog = GetNewDialog(kAboutDialogID, (Ptr)NULL, (GrafPtr)-1)) == NULL)
		return;

	CenterDialog(theDialog);
	ShowWindow(theDialog);
	DrawDialog(theDialog);
	
	while (true) 
	{
		if (GetNextEvent(mDownMask, &event))
			break;
	}

	if (theDialog != NULL)
		DisposeDialog(theDialog);
*/
}

void DoAcknowledgements()
{
	static char messageString[] = 
"Diplomacy is a trademark of Hasbro, Inc., all rights reserved.\n\n\
The Realpolitik Development Team is: \n\
\tJames M. Van Verth\n\
\tLucas B. Kruijswijk\n\
\tBen Hines\n\n\
Electronic versions of Classical, Chromatic, SailHo and Hundred\n\
by Ben Hines.  Ancient Med by Don Hessong. \n\
All other electronic versions © 2001, James M. Van Verth.\n\n\
Many thanks to all those who have contributed to Realpolitik,\n\
in particular Andy J. Williams, Jamie Dreier and Don Hessong.";

	MessageBox( mapWindow->GetNative(), messageString, 
		"Acknowledgements...", MB_OK );
/*	DialogPtr theDialog;
	Boolean done = 0;
	EventRecord  	event;
	
	if ((theDialog = GetNewDialog(kAckDialogID, (Ptr)NULL, (GrafPtr)-1)) == NULL)
		return;
	
	CenterDialog(theDialog);
	ShowWindow(theDialog);
	DrawDialog(theDialog);
	
	while (true) 
	{
		if (GetNextEvent(mDownMask, &event))
			break;
	}

	if (theDialog != NULL)
		DisposeDialog(theDialog);
*/
}


BOOL CALLBACK testProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	if (iMsg == WM_INITDIALOG)
		return TRUE;
	return FALSE; 
}

void
ShowLoadingDialog()
{
	mLoadingDialog = CreateDialog(gInstance, MAKEINTRESOURCE(IDD_LOADING), mapWindow->GetNative(), (DLGPROC)testProc);
	ShowWindow(mLoadingDialog, SW_SHOW);
}

void 
HideLoadingDialog()
{
	if (mLoadingDialog != NULL)
		DestroyWindow(mLoadingDialog);
}

/*
 * CheckSave() - opens dialog box and asks user if really wants to save
 *               returns kSaveYes if yes,
 *                       kSaveNo  if no,
 *                       kSaveCancel if cancelled
 */
short
CheckSave(void)
{
	short  theItem = MessageBox(mapWindow->GetNative(), "Save current game?", "Save (y/n)?", MB_YESNOCANCEL);
	if (theItem == IDCANCEL) 
		return kSaveCancel;		
	else if (theItem == IDYES) 
		return kSaveYes;
	else
		return kSaveNo;
}


/*
 * CheckAction() - opens dialog box and asks user if really wants to do that action
 *                 returns true if yes, false if no
 */
bool
CheckAction(char* message)
{
	short  theItem = MessageBox(mapWindow->GetNative(), message, "Warning", MB_YESNO);
	return (theItem == IDYES);
}



//countries[winner].name
//slistsize(supplies[winner])
void
WinDialog(char *message, char* name, int numSupplies)
{
	char string[256];
	sprintf(string, "%s has won with %d supply centers\n%s", name, numSupplies, message);
	MessageBox( NULL, string, kAppName, MB_ICONEXCLAMATION | MB_OK );
}


BOOL CALLBACK editSeasonProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{ 
	int code;
	char yearString[32];
	switch (iMsg)
	{
	case WM_INITDIALOG:
		sprintf(yearString, "%d", year);
		Edit_SetText(GetDlgItem(hDlg, IDC_YEAR), yearString);
		Button_SetCheck(GetDlgItem(hDlg, IDC_SPRING), (season == S_SPRING)?BST_CHECKED:BST_UNCHECKED);
		Button_SetCheck(GetDlgItem(hDlg, IDC_FALL), (season == S_FALL)?BST_CHECKED:BST_UNCHECKED);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;

	case WM_COMMAND:
		code = LOWORD(wParam);
		switch (code)
		{
		case IDOK:
			EndDialog(hDlg, 0);
			Edit_GetText(GetDlgItem(hDlg, IDC_YEAR), yearString, 31);
			sscanf(yearString, "%d", &year);
			if (Button_GetCheck(GetDlgItem(hDlg, IDC_SPRING)) == BST_CHECKED)
				season = S_SPRING;
			else
				season = S_FALL;
			gGameChanged = true;
			UpdateMapPalette();
			break;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	default:
		break;
//		return(DefDlgProc( hDlg, iMsg, wParam, lParam ));
	}
	return FALSE; 
}

void 
EditSeason( void )
{
	DialogBox(gInstance, MAKEINTRESOURCE(IDD_EDITSEASON), mapWindow->GetNative(), editSeasonProc);

}


/*
void SetWindowBounds(WindowPtr theWindow, Rect newBounds)
{
	short top;
	short left;
	short height;
	short width;
	
	short topInset;
	short leftInset;
	short bottomInset;
	short rightInset;
	
	Rect oldBounds;
	
	RgnHandle contRgn;
	RgnHandle structRgn;
	
	contRgn = ((WindowPeek) theWindow)->contRgn;
	structRgn = ((WindowPeek) theWindow)->strucRgn;
	
	oldBounds = (**structRgn).rgnBBox;
	
	if (!EqualRect(&oldBounds, &newBounds))
	{
		topInset = (**contRgn).rgnBBox.top - (**structRgn).rgnBBox.top;
		leftInset = (**contRgn).rgnBBox.left - (**structRgn).rgnBBox.left;
		bottomInset = (**contRgn).rgnBBox.bottom - (**structRgn).rgnBBox.bottom;
		rightInset = (**contRgn).rgnBBox.right - (**structRgn).rgnBBox.right;
	
		top = newBounds.top + topInset;
		left = newBounds.left + leftInset;
		height = newBounds.bottom - top - bottomInset;
		width = newBounds.right - left - rightInset;
		
		HideWindow(theWindow);
		MoveWindow(theWindow, left, top, FALSE);
		SizeWindow(theWindow, width, height, TRUE);
		SelectWindow(theWindow);
		ShowWindow(theWindow);
	}
}


Rect GetWindowContentRect(WindowPtr window)
{
	WindowPtr oldPort;
	Rect contentRect;
	
	GetPort(&oldPort);
	SetPort(window);
	contentRect = window->portRect;
	LocalToGlobalRect(&contentRect);
	SetPort(oldPort);
	
	return contentRect;
}


Rect GetWindowStructureRect(WindowPtr window)
{
	const short kOffscreenLocation = 0x4000;
	
	GrafPtr oldPort;
	Rect structureRect;
	Point windowLoc;
	
	if (((WindowPeek)window)->visible)
		structureRect = (*(((WindowPeek)window)->strucRgn))->rgnBBox;
	else
	{
		GetPort(&oldPort);
		SetPort(window);
		windowLoc = GetGlobalTopLeft(window);
		MoveWindow(window, windowLoc.h, kOffscreenLocation, FALSE);
		ShowHide(window, TRUE);
		structureREct = (*(((WindowPeek)window)->strucRgn))->rgnBBox;
		ShowHide(window, FALSE);
		MoveWindow(window, windowLoc.h, windowLoc.v, FALSE);
		OffsetRect(&structureRect, 0, windowLoc.v - kOffscreenLocation);
		SetPort(&oldPort);
	}
	return structureRect;
}
*/		
