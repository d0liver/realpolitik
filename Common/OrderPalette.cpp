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
#include <string.h>
#include "OrderPalette.h"
#include "EditMap.h"
#include "Game.h"
#include "MapData.h"
#include "MapDisplay.h"
#include "Order.h"
#include "Strings.h"

//---------------------------------------------------------------------------------
//
// Note: I am using O_BUILD to mean "build a fleet" and O_WAIVE to mean "build an army"
//
//---------------------------------------------------------------------------------
OrderPalette::OrderPalette() 
{
	mOrderMode = O_MOVE;
	mActive = true;
}


OrderPalette::~OrderPalette()
{

}

//-------------------------------------------------------------------------------
// Palette code
//-------------------------------------------------------------------------------
void
OrderPalette::Draw()
{
	mRect.left += 100;
	DrawSeason();

	if (dislodges)
	{
		if (mActive && !gCommit && !winner)
			DrawDislodgePalette();
		else
			DrawGreyDislodgePalette();
	}
	else if (adjustments)
		if (mActive && !gCommit && !winner)
			DrawAdjustPalette();
		else
			DrawGreyAdjustPalette();
	else
		if (mActive && !gCommit && !winner)
			DrawMovePalette();
		else
			DrawGreyMovePalette(); 
	mRect.left -= 100;
}

void
OrderPalette::DrawSeason()
{
	short h, v;
	char string[256], yearstring[256];
	Rect clearRect;
	
	clearRect.left = mRect.left-100;
	clearRect.right = mRect.left;
	clearRect.top = mRect.top;
	clearRect.bottom = mRect.bottom; 
	EraseRect(&clearRect);

	if (season == S_NONE)
		return;
	
	h = mRect.left-98;
	v = mRect.bottom-2;
	MoveTo(h, v);
	
	if (EditModeOn())
	{
		TextFace( italic );
	}
	
	GetIndCString(string, kSeasonStringsID, season);
	strcat(string, " ");
	NumToString(year, (unsigned char*)yearstring);
	strcat(string, PtoCstr((unsigned char*)yearstring));
	DrawString(CtoPstr(string));
	TextFace(normal);	
}

void
OrderPalette::DrawMovePalette()
{
	Rect clearRect, circleRect;
	PolyHandle arrow;
	short v;
	
	PenNormal();
	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	if (mOrderMode == O_MOVE)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);
	
	InsetRect(&mRect, -1, -1);
	FrameRect(&mRect);
	InsetRect(&mRect, 1, 1);

	MoveTo(mRect.left-1, clearRect.top);
	LineTo(mRect.left-1, clearRect.bottom);
	
	if (mOrderMode == O_MOVE)
		PenPat(&qd.white);
	else
		PenNormal();
	v = mRect.bottom - 7;	
	arrow = OpenPoly();
	MoveTo(-5,4);
	LineTo(2,0);
	LineTo(-5,-4);
	LineTo(-5,4);
	ClosePoly();
	OffsetPoly(arrow, mRect.left+12, v);
	PaintPoly(arrow);
	KillPoly(arrow);
	MoveTo(mRect.left+2,v);
	LineTo(mRect.left+6,v);
	
	clearRect.left = mRect.left+17;
	clearRect.right = mRect.left+33;
	if (mOrderMode == O_SUPP)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left+16, (short)clearRect.top);
	LineTo(mRect.left+16, (short)clearRect.bottom);
	
	if (mOrderMode == O_SUPP)
		PenPat(&qd.white);
	else
		PenNormal();

	::SetRect(&circleRect, -3, -3, 3, 3);
	OffsetRect(&circleRect, mRect.left+25, v);
	PaintOval(&circleRect);	
	clearRect.left = mRect.left+34;
	clearRect.right = mRect.left+49;
	if (mOrderMode == O_CONV)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left+33, (short)clearRect.top);
	LineTo(mRect.left+33, (short)clearRect.bottom);
	
	if (mOrderMode == O_CONV)
		PenPat(&qd.white);
	else
		PenNormal();
	::SetRect(&circleRect, -4, -4, 4, 4);
	OffsetRect(&circleRect, mRect.left+42, v);	
	FrameOval(&circleRect);
	InsetRect(&circleRect, 2, 2);
	FrameArc(&circleRect, 45, -310);
	
	PenNormal();
}


void
OrderPalette::DrawGreyMovePalette()
{
	Rect clearRect, circleRect;
	PolyHandle arrow;
	short v;
	
	PenNormal();
	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	EraseRect(&clearRect);	
	
	PenPat(&qd.black);
	MoveTo(99, (short)clearRect.top);
	LineTo(99, (short)clearRect.bottom);
	
	PenPat(&qd.gray);
	v = mRect.bottom - 7;	
	arrow = OpenPoly();
	MoveTo(-5,4);
	LineTo(2,0);
	LineTo(-5,-4);
	LineTo(-5,4);
	ClosePoly();
	OffsetPoly(arrow, mRect.left+12, v);
	PaintPoly(arrow);
	KillPoly(arrow);
	MoveTo(102,v);
	LineTo(106,v);
	
	clearRect.left = 117;
	clearRect.right = 133;
	EraseRect(&clearRect);	
	
	PenPat(&qd.black);
	MoveTo(116, clearRect.top);
	LineTo(116, clearRect.bottom);
	
	PenPat(&qd.gray);
	::SetRect(&circleRect, -3, -3, 3, 3);
	OffsetRect(&circleRect, mRect.left+25, v);
	PaintOval(&circleRect);
	
	clearRect.left = 134;
	clearRect.right = 149;
	EraseRect(&clearRect);	
	
	PenPat(&qd.black);
	MoveTo(133, clearRect.top);
	LineTo(133, clearRect.bottom);
	
	PenPat(&qd.gray);
	::SetRect(&circleRect, -4, -4, 4, 4);
	OffsetRect(&circleRect, mRect.left+42, v);	
	FrameOval(&circleRect);
	InsetRect(&circleRect, 2, 2);
	FrameArc(&circleRect, 45, -310);
	
	PenNormal();

}

void
OrderPalette::DrawAdjustPalette()
{
	Rect clearRect, circleRect;
	PolyHandle base;
	short v;
	
	PenNormal();

	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	if (mOrderMode == O_BUILD)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left-1, clearRect.top);
	LineTo(mRect.left-1, clearRect.bottom);
	
	if (mOrderMode == O_BUILD)
		PenPat(&qd.white);
	else
		PenNormal();
	v = (short)mRect.bottom-7;	
	base = OpenPoly();
	MoveTo(-7,-1);
	LineTo(7,-1);
	LineTo(3,3);
	LineTo(-3,3);
	LineTo(-7,-1);
	ClosePoly();
	OffsetPoly(base, mRect.left+8, v);
	PaintPoly(base);
	KillPoly(base);
	PenSize(2,2);
	MoveTo(105,v);
	LineTo(105,v-4); 
	MoveTo(110,v);
	LineTo(110,v-4); 
	PenSize(1,1);
	
	clearRect.left = mRect.left+17;
	clearRect.right = mRect.left+33;
	if (mOrderMode == O_WAIVE)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left+16, (short)clearRect.top);
	LineTo(mRect.left+16, (short)clearRect.bottom);
	
	if (mOrderMode == O_WAIVE)
		PenPat(&qd.white);
	else
		PenNormal();
	PenSize(2,2);
	MoveTo(mRect.left+19, v-3);
	LineTo(mRect.left+26, v-3);
	PenSize(1,1);
	MoveTo(mRect.left+26, v+2);
	LineTo(mRect.left+30, v+2);
	::SetRect(&circleRect, -3, -3, 3, 3);
	OffsetRect(&circleRect, mRect.left+26, v);
	EraseOval(&circleRect);
	FrameOval(&circleRect);

	PenNormal();
	clearRect.left = mRect.left+34;
	clearRect.right = mRect.left+49;
	if (mOrderMode == O_REMOVE)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left+33, (short)clearRect.top);
	LineTo(mRect.left+33, (short)clearRect.bottom);
	
	if (mOrderMode == O_REMOVE)
		PenPat(&qd.white);
	else
		PenNormal();
	PenSize(2,2);
	::SetRect(&circleRect, -5, -5, 3, 3);
	OffsetRect(&circleRect, mRect.left+42, v);
	MoveTo((short)circleRect.left, (short)circleRect.top);
	LineTo((short)circleRect.right, (short)circleRect.bottom);
	MoveTo((short)circleRect.right, (short)circleRect.top);
	LineTo((short)circleRect.left, (short)circleRect.bottom);
	
	PenNormal();

}


void
OrderPalette::DrawGreyAdjustPalette()
{
	Rect clearRect, circleRect;
	PolyHandle base;
	short v;
	
	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left-1, clearRect.top);
	LineTo(mRect.left-1, clearRect.bottom);
	
	PenPat(&qd.gray);
	v = mRect.bottom - 7;	
	base = OpenPoly();
	MoveTo(-7,-1);
	LineTo(7,-1);
	LineTo(3,3);
	LineTo(-3,3);
	LineTo(-7,-1);
	ClosePoly();
	OffsetPoly(base, mRect.left+8, v);
	PaintPoly(base);
	KillPoly(base);
	PenSize(2,2);
	MoveTo(mRect.left+5,v);
	LineTo(mRect.left+5,v-4); 
	MoveTo(mRect.left+10,v);
	LineTo(mRect.left+10,v-4); 
	PenSize(1,1);
	
	clearRect.left = mRect.left+17;
	clearRect.right = mRect.left+33;
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left+16, clearRect.top);
	LineTo(mRect.left+16, clearRect.bottom);
	
	PenPat(&qd.gray);
	PenSize(2,2);
	MoveTo(mRect.left+19, v-3);
	LineTo(mRect.left+26, v-3);
	PenSize(1,1);
	MoveTo(mRect.left+26, v+2);
	LineTo(mRect.left+30, v+2);
	::SetRect(&circleRect, -3, -3, 3, 3);
	OffsetRect(&circleRect, mRect.left+26, v);
	EraseOval(&circleRect);
	FrameOval(&circleRect);
	
	clearRect.left = mRect.left+34;
	clearRect.right = mRect.left+49;
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left+33, clearRect.top);
	LineTo(mRect.left+33, clearRect.bottom);
	
	PenPat(&qd.gray);
	PenSize(2,2);
	::SetRect(&circleRect, -5, -5, 3, 3);
	OffsetRect(&circleRect, mRect.left+42, v);
	MoveTo(circleRect.left, circleRect.top);
	LineTo(circleRect.right, circleRect.bottom);
	MoveTo(circleRect.right, circleRect.top);
	LineTo(circleRect.left, circleRect.bottom);
	
	PenNormal();

}

void
OrderPalette::DrawDislodgePalette()
{
	Rect clearRect, circleRect;
	PolyHandle arrow;
	short v;
	
	PenPat(&qd.black);
	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	if (mOrderMode == O_RETR)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left-1, clearRect.top);
	LineTo(mRect.left-1, clearRect.bottom);
	
	if (mOrderMode == O_RETR)
		PenPat(&qd.white);
	else
		PenNormal();
	v = mRect.bottom-7;	
	arrow = OpenPoly();
	MoveTo(-5,4);
	LineTo(2,0);
	LineTo(-5,-4);
	LineTo(-5,4);
	ClosePoly();
	OffsetPoly(arrow, mRect.left+12, v);
	PaintPoly(arrow);
	KillPoly(arrow);
	MoveTo(mRect.left+2,v);
	LineTo(mRect.left+6,v);
	
	clearRect.left = mRect.left+17;
	clearRect.right = mRect.left+33;
	if (mOrderMode == O_REMOVE)
		PaintRect(&clearRect);
	else
		EraseRect(&clearRect);	
	
	MoveTo(mRect.left+16, clearRect.top);
	LineTo(mRect.left+16, clearRect.bottom);
	
	if (mOrderMode == O_REMOVE)
		PenPat(&qd.white);
	else
		PenNormal();
	PenSize(2,2);
	::SetRect(&circleRect, -5, -5, 3, 3);
	OffsetRect(&circleRect, mRect.left+25, v);
	MoveTo(circleRect.left, circleRect.top);
	LineTo(circleRect.right, circleRect.bottom);
	MoveTo(circleRect.right, circleRect.top);
	LineTo(circleRect.left, circleRect.bottom);
	
	clearRect.left = mRect.left+34;
	clearRect.right = mRect.left+49;
	EraseRect(&clearRect);
		
	
	PenNormal();
	MoveTo(mRect.left+33, clearRect.top);
	LineTo(mRect.left+33, clearRect.bottom);
		
}


void
OrderPalette::DrawGreyDislodgePalette()
{
	Rect clearRect, circleRect;
	PolyHandle arrow;
	short v;
	
	clearRect.bottom = mRect.bottom;
	clearRect.top = mRect.top;
	clearRect.left = mRect.left;
	clearRect.right = mRect.left+16;
	
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left-1, clearRect.top);
	LineTo(mRect.left-1, clearRect.bottom);
	
	PenPat(&qd.gray);
	v = mRect.bottom-7;	
	arrow = OpenPoly();
	MoveTo(-5,4);
	LineTo(2,0);
	LineTo(-5,-4);
	LineTo(-5,4);
	ClosePoly();
	OffsetPoly(arrow, mRect.left+12, v);
	PaintPoly(arrow);
	KillPoly(arrow);
	MoveTo(102,v);
	LineTo(106,v);
	
	clearRect.left = mRect.left+17;
	clearRect.right = mRect.left+33;
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left+16, clearRect.top);
	LineTo(mRect.left+16, clearRect.bottom);
	
	PenPat(&qd.gray);
	PenSize(2,2);
	::SetRect(&circleRect, -5, -5, 3, 3);
	OffsetRect(&circleRect, mRect.left+25, v);
	MoveTo(circleRect.left, circleRect.top);
	LineTo(circleRect.right, circleRect.bottom);
	MoveTo(circleRect.right, circleRect.top);
	LineTo(circleRect.left, circleRect.bottom);
	
	clearRect.left = mRect.left+34;
	clearRect.right = mRect.left+49;
	EraseRect(&clearRect);	
	
	PenNormal();
	MoveTo(mRect.left+33, clearRect.top);
	LineTo(mRect.left+33, clearRect.bottom);
	
}

void
OrderPalette::Reset()
{
	if (dislodges)
		mOrderMode = O_RETR;
	else if (adjustments)
		mOrderMode = O_BUILD;
	else
		mOrderMode = O_MOVE;
}

void
OrderPalette::SetRect(const ScrollableWindow* theWindow)
{
	int height = theWindow->GetHScrollHeight();
	::SetRect(&mRect, 
			0, 
			height - 14, 
			149, 
			height);
}

void
OrderPalette::Activate(ScrollableWindow*, bool activating)
{
	if (activating)
	{
		mActive = true;
		UpdateMapPalette();
	}
	else
	{
		mActive = false;
		UpdateMapPalette();
	}
}
	

bool
OrderPalette::InPalette(Point where)
{
	return (where.h >= mRect.left && where.h <= mRect.right 
			&& where.v >= mRect.top && where.v <= mRect.bottom);
}

short
OrderPalette::DoContent(const Point& where)
{
	short oldmode = mOrderMode;

	if (!mActive || gCommit)
		return oldmode;
		
	if (dislodges)
		DoDislodgePalette(where);
	else if (adjustments)
		DoAdjustPalette(where);
	else
		DoMovePalette(where);

	return oldmode;
}

short
OrderPalette::DoShiftPalette(bool left)
{
	short oldmode = mOrderMode;
	
	if (!mActive || gCommit)
		return oldmode;

	if (dislodges)
		switch(mOrderMode) 
		{
		case O_RETR:
			mOrderMode = O_REMOVE;
			break;			
		case O_REMOVE:
			mOrderMode = O_RETR;
			break;
		}
	else if (adjustments)
		switch(mOrderMode)
		{
		case O_BUILD:
			if (left)
				mOrderMode = O_REMOVE;
			else
				mOrderMode = O_WAIVE;
			break;
		case O_WAIVE:
			if (left)
				mOrderMode = O_BUILD;
			else
				mOrderMode = O_REMOVE;
			break;
		case O_REMOVE:
			if (left)
				mOrderMode = O_WAIVE;
			else
				mOrderMode = O_BUILD;
			break;
		}
	else 
		switch(mOrderMode)
		{
		case O_MOVE:
			if (left)
				mOrderMode = O_CONV;
			else
				mOrderMode= O_SUPP;
			break;
		case O_SUPP:
			if (left)
				mOrderMode = O_MOVE;
			else
				mOrderMode= O_CONV;
			break;
		case O_CONV:
			if (left)
				mOrderMode = O_SUPP;
			else
				mOrderMode = O_MOVE;
			break;
		}
	
	UpdateMapPalette();

	return oldmode;
}


bool
OrderPalette::SetOrderMode( short mode )
{
	short oldmode = mOrderMode;
	
	if (!mActive || gCommit)
		return false;

	if (dislodges)
	{
		if ( mode == O_RETR || mode == O_REMOVE )
		{
			mOrderMode = mode;
		}
		else
		{
			return false;
		}
	}
	else if (adjustments)
	{
		if ( mode == O_BUILD || mode == O_WAIVE || mode == O_REMOVE )
		{
			mOrderMode = mode;
		}
		else
		{
			return false;
		}
	}
	else 
	{
		if ( mode == O_MOVE || mode == O_SUPP || mode == O_CONV )
		{
			mOrderMode = mode;
		}
		else
		{
			return false;
		}
	}
	
	UpdateMapPalette();

	return true;
}


void
OrderPalette::DoDislodgePalette(Point where)
{
	if (where.h >= 133)
		return;
		
	if (where.h < 116)
		mOrderMode = O_RETR;
	else if (where.h < 133)
		mOrderMode = O_REMOVE;
}

void
OrderPalette::DoMovePalette(Point where)
{
	if (where.h < 116)
		mOrderMode = O_MOVE;
	else if (where.h < 133)
		mOrderMode = O_SUPP;
	else
		mOrderMode = O_CONV; 
}

void
OrderPalette::DoAdjustPalette(Point where)
{
	if (where.h < 116)
		mOrderMode = O_BUILD;
	else if (where.h < 133)
		mOrderMode = O_WAIVE;
	else
		mOrderMode = O_REMOVE; 
}
