/*
 *  Copyright (C) 2003  The Pentagram Team
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
#include "ModalGump.h"
#include "Kernel.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ModalGump,Gump);

ModalGump::ModalGump() : Gump()
{

}


ModalGump::ModalGump(int x, int y, int width, int height, uint16 owner,
					 uint32 _Flags, sint32 layer)
	: Gump(x, y, width, height, owner, _Flags, layer)
{

}

ModalGump::~ModalGump()
{

}

void ModalGump::InitGump()
{
	Gump::InitGump();

	// lock keyboard here?

	Kernel::get_instance()->pause();
}

Gump* ModalGump::FindGump(int mx, int my)
{
	Gump* ret = Gump::FindGump(mx, my);
	if (!ret) ret = this; // we take all mouse input

	return ret;
}

uint16 ModalGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx, my);
	if (!objid) objid = getObjId();

	return objid;
}

void ModalGump::Close(bool no_del)
{
	// free keyboard here?

	Kernel::get_instance()->unpause();

	Gump::Close(no_del);
}

Gump* ModalGump::OnMouseDown(int button, int mx, int my)
{
	Gump* handled = Gump::OnMouseDown(button, mx, my);
	if (!handled) handled = this;
	return handled;
}


void ModalGump::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Gump::saveData(ods);
}

bool ModalGump::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Gump::loadData(ids)) return false;

	return true;
}