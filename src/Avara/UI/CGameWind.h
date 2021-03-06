/*
    Copyright �1994-1995, Juri Munkki
    All rights reserved.

    File: CGameWind.h
    Created: Wednesday, November 16, 1994, 2:00
    Modified: Thursday, June 8, 1995, 8:03
*/

#include "CWindowCommander.h"

class	CGameWind : public CWindowCommander
{
public:
	virtual	void				IWindowCommander(CCommander *theCommander,
												CWindowCommander **theList);
												
	virtual	void				PositionWindow();
	virtual	void				DrawContents();
	virtual	Boolean				DoGrow(EventRecord *theEvent);
	virtual	CloseRequestResult	CloseRequest(Boolean isQuitting);
};