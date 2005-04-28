/*
 *  Copyright (C) 2005  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef U8SAVEGUMP_H
#define U8SAVEGUMP_H

#include "Gump.h"

// U8-style load/save gump.

class EditWidget;

class U8SaveGump : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	U8SaveGump(bool save, int page);
	virtual ~U8SaveGump();

	virtual void InitGump();
	virtual void Close(bool no_del=false);

	virtual Gump* OnMouseDown(int button, int mx, int my);
	virtual void OnMouseClick(int button, int mx, int my);
	virtual bool OnKeyDown(int key, int mod);
	virtual void ChildNotify(Gump *child, uint32 message);

protected:
	bool save;
	int page;

	std::vector<EditWidget*> editwidgets;
	std::vector<std::string> descriptions;

	void loadDescriptions();

	bool loadgame(int index);
	bool savegame(int index, const std::string& name);
};


#endif