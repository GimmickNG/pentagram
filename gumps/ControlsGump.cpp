/*
 *  Copyright (C) 2004  The Pentagram Team
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

#include "pent_include.h"
#include "ControlsGump.h"

#include "RenderSurface.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"
#include "QuitGump.h"
#include "BindGump.h"

#include "GUIApp.h"
#include "HIDManager.h"
#include "ObjectManager.h"

#include "IDataSource.h"
#include "ODataSource.h"

static const int font = 9;

class ControlEntryGump : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();
	ControlEntryGump(int x, int y, int width, char * binding, char * name);
	virtual ~ControlEntryGump(void);
	virtual void InitGump();
	virtual void ChildNotify(Gump *child, uint32 message);
protected:
	Pentagram::istring bindingName;
	std::string displayedName;
	Gump * button;
};

DEFINE_RUNTIME_CLASSTYPE_CODE(ControlEntryGump,Gump);

ControlEntryGump::ControlEntryGump(int x, int y, int width, char * binding, char * name)
	:Gump(x, y, width, 5), bindingName(binding), displayedName(name)
{
}

ControlEntryGump::~ControlEntryGump()
{
}

void ControlEntryGump::InitGump()
{
	// close all children so we can simply use this method to re init
	std::list<Gump*>::iterator it;
	for (it = children.begin(); it != children.end(); ++it)
	{
		Gump *g = *it;
		if (! g->IsClosing())
			g->Close();
	}

	std::vector<const char *> controls;
	std::vector<const char *>::iterator i;
	HIDManager * hidmanager = HIDManager::get_instance();
	Gump * widget;

	Pentagram::Rect rect;
	button = new ButtonWidget(0, 0, displayedName, font);
	button->InitGump();
	AddChild(button);
	button->GetDims(rect);

	dims.h = rect.h;

	hidmanager->getBindings(bindingName, controls);
	int x = 120;
	for (i = controls.begin(); i != controls.end(); ++i)
	{
		widget = new TextWidget(x, 0, *i, font);
		widget->InitGump();
		AddChild(widget, false);
		widget->GetDims(rect);
		x += rect.w + 5;
	}
}

void ControlEntryGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		if (cid == button->getObjId())
		{
			Gump* desktopGump = GUIApp::get_instance()->getDesktopGump();
			ModalGump* gump = new BindGump(&bindingName, parent);
			gump->InitGump();
			desktopGump->AddChild(gump);
			gump->setRelativePosition(CENTER);
		}
	}
	else if (message == BindGump::UPDATE)
	{
		parent->ChildNotify(child, message);
	}
}

DEFINE_RUNTIME_CLASSTYPE_CODE(ControlsGump,Gump);

ControlsGump::ControlsGump(): Gump(0, 0, 5, 5)
{
	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	app->setMouseCursor(GUIApp::MOUSE_HAND);
}

ControlsGump::~ControlsGump()
{
	GUIApp::get_instance()->popMouseCursor();
}

void ControlsGump::InitGump()
{
	Gump::InitGump();

	dims.w = 220;
	dims.h = 120;

	Gump * widget = new TextWidget(0, 0, "Controls", font);
	widget->InitGump();
	AddChild(widget, false);
	widget = new TextWidget(120, 0, "Keys", font);
	widget->InitGump();
	AddChild(widget, false);
}

void ControlsGump::addEntry(char * binding, char * name, int & x, int & y)
{
	Pentagram::Rect rect;
	Gump * widget = new ControlEntryGump(x, y, dims.w - x, binding, name);
	widget->InitGump();
	AddChild(widget);
	widget->GetDims(rect);
	y += rect.h;
}

void ControlsGump::ChildNotify(Gump *child, uint32 message)
{
	if (message == BindGump::UPDATE)
	{
		std::list<Gump*>::iterator it;
		for (it = children.begin(); it != children.end(); ++it)
		{
			ControlEntryGump *g =  p_dynamic_cast<ControlEntryGump*>(*it);
			if (g)
				g->InitGump();
		}
	}
}

void ControlsGump::PaintThis(RenderSurface* surf, sint32 lerp_factor)
{
	Gump::PaintThis(surf, lerp_factor);
}

bool ControlsGump::OnKeyDown(int key, int mod)
{
	switch (key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

//static
Gump * ControlsGump::showEngineMenu()
{
	ControlsGump* gump = new ControlsGump();
	gump->InitGump();
	int x = 4;
	int y = 12;
	gump->addEntry("quickSave", "Quick Save", x, y);
	gump->addEntry("quickLoad", "Quick Save", x, y);
	gump->addEntry("avatarInStatis", "Statis Mode", x, y);
	gump->addEntry("engineStats", "Engine Stats", x, y);
	gump->addEntry("paintEditorItems", "View Editor Items", x, y);
	gump->addEntry("itemLocator", "Locate Item", x, y);
	gump->addEntry("toggleFrameByFrame", "Single Frame Mode", x, y);
	gump->addEntry("advanceFrameByFrame", "Next Frame", x, y);
	gump->addEntry("toggleConsole", "Console", x, y);
	return gump;
}

Gump * ControlsGump::showU8Menu()
{
	ControlsGump* gump = new ControlsGump();
	gump->InitGump();
	int x = 4;
	int y = 12;
	gump->addEntry("toggleCombat", "Combat Mode", x, y);
	gump->addEntry("openInventory", "Open Inventory", x, y);
	gump->addEntry("openBackpack", "Open Backpack", x, y);
	gump->addEntry("recall", "Recall", x, y);
	gump->addEntry("runFirstEgg", "First Scene", x, y);
	gump->addEntry("runExecutionEgg", "Execution Scene", x, y);
	gump->addEntry("u8ShapeViewer", "Shape Viewer", x, y);
	gump->addEntry("showMenu", "Menu", x, y);
	gump->addEntry("quit", "Quit", x, y);
	return gump;
}

bool ControlsGump::loadData(IDataSource* ids)
{
	return true;
}

void ControlsGump::saveData(ODataSource* ods)
{
}
