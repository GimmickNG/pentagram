/*
Copyright (C) 2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "StartU8Process.h"
#include "Game.h"
#include "LoopScript.h"
#include "UCList.h"
#include "CurrentMap.h"
#include "Egg.h"
#include "CameraProcess.h"
#include "World.h"
#include "GUIApp.h"
#include "Kernel.h"
#include "MenuGump.h"
#include "SettingManager.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(StartU8Process,Process);

StartU8Process::StartU8Process(const std::string &savename_) : Process(), init(false), savename(savename_)
{
	SettingManager::get_instance()->get("skipstart", skipstart);
}


void StartU8Process::run()
{
	if (!skipstart && !init) {
		init = true;
		ProcId moviepid = Game::get_instance()->playIntroMovie();
		Process* movieproc = Kernel::get_instance()->getProcess(moviepid);
		if (movieproc) {
			waitFor(movieproc);
			return;
		}
	}

	// Try to load the save game, if succeeded this pointer will no longer be valid
	if (!savename.empty() && GUIApp::get_instance()->loadGame(savename)) {
		return;
	}

	CurrentMap* currentmap = World::get_instance()->getCurrentMap();
	UCList uclist(2);

	if (!skipstart) {
		LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(36)));
		currentmap->areaSearch(&uclist, script, sizeof(script),
							   0, 256, false, 16188, 7500);
		if (uclist.getSize() < 1) {
			perr << "Unable to find FIRST egg!" << std::endl;
			return;
		}
	
		uint16 objid = uclist.getuint16(0);
		Egg* egg = p_dynamic_cast<Egg*>(getObject(objid));
		sint32 ix, iy, iz;
		egg->getLocation(ix,iy,iz);
		// Center on egg
		CameraProcess::SetCameraProcess(new CameraProcess(ix,iy,iz));
		egg->hatch();
	}

	// Music Egg
	// Item 2145 (class Item, shape 562, 0, (11551,2079,48) q:52, m:0, n:0, f:2000, ef:2)
	uclist.free();
	LOOPSCRIPT(musicscript, LS_SHAPE_EQUAL1(562));
	currentmap->areaSearch(&uclist, musicscript, sizeof(musicscript),
						   0, 256, false, 11551, 2079);

	if (uclist.getSize() < 1) {
		perr << "Unable to find MUSIC egg!" << std::endl;
	}
	else {
		ObjId objid = uclist.getuint16(0);
		Item *musicEgg = getItem(objid);
		musicEgg->callUsecodeEvent_cachein();
	}

	if (!skipstart)
		MenuGump::inputName();
	else
		GUIApp::get_instance()->setAvatarInStasis(false);


	terminate();
}

void StartU8Process::saveData(ODataSource* ods)
{
	CANT_HAPPEN();

	Process::saveData(ods);
}

bool StartU8Process::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	return true;
}
