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
#include "TargetGump.h"

#include "GUIApp.h"
#include "GumpNotifyProcess.h"
#include "Item.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(TargetGump,ModalGump);

TargetGump::TargetGump() : ModalGump(), target_tracing(false)
{

}


TargetGump::TargetGump(int x, int y)
	: ModalGump(x, y, 0, 0), target_tracing(false)
{

}

TargetGump::~TargetGump()
{

}

void TargetGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	// we're invisible
}

void TargetGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);

	// we need a notifier process
	CreateNotifier();

	GUIApp* guiapp = GUIApp::get_instance();
	guiapp->pushMouseCursor();
	guiapp->setMouseCursor(GUIApp::MOUSE_TARGET);
}

void TargetGump::Close(bool no_del)
{
	GUIApp* guiapp = GUIApp::get_instance();
	guiapp->popMouseCursor();

	ModalGump::Close(no_del);
}

bool TargetGump::PointOnGump(int mx, int my)
{
	// HACK alert: if we're currently tracing from TargetGump::OnMouseUp,
	//  then we do NOT want to intercept the trace
	if (target_tracing) return false;

	return ModalGump::PointOnGump(mx, my);
}

void TargetGump::OnMouseUp(int button, int mx, int my)
{
	target_tracing = true;

	parent->GumpToScreenSpace(mx, my);

	Gump* desktopgump = parent;
	ObjId objid = desktopgump->TraceObjId(mx, my);
	Item *item = getItem(objid);

	if (item) {
		// done
		pout << "Target result: ";
		item->dumpInfo();

		process_result = objid;
		Close();
	}

	target_tracing = false;
}

uint32 TargetGump::I_target(const uint8* /*args*/, unsigned int /*argsize*/)
{
	TargetGump* targetgump = new TargetGump(0, 0);
	targetgump->InitGump(0);

	return targetgump->GetNotifyProcess()->getPid();
}



void TargetGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool TargetGump::loadData(IDataSource* ids, uint32 versin)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}
