/*
 *  Copyright (C) 2004-2005  The Pentagram Team
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
#include "QuitGump.h"

#include "GameData.h"
#include "GumpShapeArchive.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "GUIApp.h"
#include "DesktopGump.h"
#include "ButtonWidget.h"
#include "TextWidget.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(QuitGump,ModalGump);

QuitGump::QuitGump(): ModalGump(0, 0, 5, 5)
{
	GUIApp * app = GUIApp::get_instance();
	app->pushMouseCursor();
	app->setMouseCursor(GUIApp::MOUSE_HAND);
}

QuitGump::~QuitGump()
{
	GUIApp::get_instance()->popMouseCursor();
}

static const int gumpShape = 17;
static const int askShapeId = 18;
static const int yesShapeId = 47;
static const int noShapeId = 50;

void QuitGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);

	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	FrameID askshape(GameData::GUMPS, askShapeId, 0);
	askshape = _TL_SHP_(askshape);

	Shape* askShape = GameData::get_instance()->getShape(askshape);
	sf = askShape->getFrame(askshape.framenum);
	assert(sf);

	Gump * ask = new Gump(0, 0, sf->width, sf->height);
	ask->SetShape(askShape, askshape.framenum);
	ask->InitGump(this);
	ask->setRelativePosition(TOP_CENTER, 0, 5);

	FrameID yesbutton_up(GameData::GUMPS, yesShapeId, 0);
	FrameID yesbutton_down(GameData::GUMPS, yesShapeId, 1);
	yesbutton_up = _TL_SHP_(yesbutton_up);
	yesbutton_down = _TL_SHP_(yesbutton_down);

	Gump * widget;
	widget = new ButtonWidget(0, 0, yesbutton_up, yesbutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_LEFT, 16, 38);
	yesWidget = widget->getObjId();

	FrameID nobutton_up(GameData::GUMPS, noShapeId, 0);
	FrameID nobutton_down(GameData::GUMPS, noShapeId, 1);
	nobutton_up = _TL_SHP_(nobutton_up);
	nobutton_down = _TL_SHP_(nobutton_down);

	widget = new ButtonWidget(0, 0, nobutton_up, nobutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_RIGHT, -16, 38);
	noWidget = widget->getObjId();
}


void QuitGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool QuitGump::OnKeyDown(int key, int mod)
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

void QuitGump::ChildNotify(Gump *child, uint32 message)
{
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK)
	{
		if (cid == yesWidget)
		{
			GUIApp::get_instance()->ForceQuit();
		}
		else if (cid == noWidget)
		{
			Close();
		}
	}
}

bool QuitGump::OnTextInput(int unicode)
{
	if (!(unicode & 0xFF80)) {
		char c = unicode & 0x7F;
		if (_TL_("Yy").find(c) != std::string::npos) {
			GUIApp::get_instance()->ForceQuit();
		} else if (_TL_("Nn").find(c) != std::string::npos) {
			Close();
		}
	}
	return true;
}

//static
void QuitGump::verifyQuit()
{
	ModalGump* gump = new QuitGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

void QuitGump::ConCmd_verifyQuit(const Console::ArgvType &argv)
{
	QuitGump::verifyQuit();
}

bool QuitGump::loadData(IDataSource* ids)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void QuitGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

