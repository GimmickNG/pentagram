/*
Copyright (C) 2003-2006 The Pentagram team

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

#include "ItemFactory.h"
#include "GameData.h"
#include "MainShapeArchive.h"
#include "ShapeInfo.h"
#include "Item.h"
#include "Container.h"
#include "Actor.h"
#include "MainActor.h"
#include "GlobEgg.h"
#include "Egg.h"
#include "MonsterEgg.h"
#include "TeleportEgg.h"
#include "CoreApp.h"
#include "IDataSource.h"

Item* ItemFactory::createItem(uint32 shape, uint32 frame, uint16 quality,
							  uint16 flags, uint16 npcnum, uint16 mapnum,
							  uint32 extendedflags, bool objid)
{
	// check what class to create
	ShapeInfo *info = GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
	if (info == 0) return 0;

	// New item, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	uint32 family = info->family;

	switch (family) {
	case ShapeInfo::SF_GENERIC:
	case ShapeInfo::SF_QUALITY:
	case ShapeInfo::SF_QUANTITY:
	case ShapeInfo::SF_BREAKABLE:
	case ShapeInfo::SF_REAGENT: // reagents need special handling too
	case ShapeInfo::SF_15: // what's this?
	{
		// 'simple' item

		Item* item = new Item();
		item->setShape(shape);
		item->frame = frame;
		item->quality = quality;
		item->flags = flags;
		item->npcnum = npcnum;
		item->mapnum = mapnum;
		item->extendedflags = extendedflags;
		if (objid) item->assignObjId();
		return item;
	}

	case ShapeInfo::SF_CONTAINER:
	{
		// container

		Container* container = new Container();
		container->setShape(shape);
		container->frame = frame;
		container->quality = quality;
		container->flags = flags;
		container->npcnum = npcnum;
		container->mapnum = mapnum;
		container->extendedflags = extendedflags;
		if (objid) container->assignObjId();
		return container;
	}

	case ShapeInfo::SF_GLOBEGG:
	{
		// glob egg

		GlobEgg* globegg = new GlobEgg();
		globegg->setShape(shape);
		globegg->frame = frame;
		globegg->quality = quality;
		globegg->flags = flags;
		globegg->npcnum = npcnum;
		globegg->mapnum = mapnum;
		globegg->extendedflags = extendedflags;
		if (objid) globegg->assignObjId();
		return globegg;
	}

	case ShapeInfo::SF_UNKEGG:
	{
		Egg* egg = new Egg();
		egg->setShape(shape);
		egg->frame = frame;
		egg->quality = quality;
		egg->flags = flags;
		egg->npcnum = npcnum;
		egg->mapnum = mapnum;
		egg->extendedflags = extendedflags;
		if (objid) egg->assignObjId();
		return egg;
	}

	case ShapeInfo::SF_MONSTEREGG:
	{
		MonsterEgg* egg = new MonsterEgg();
		egg->setShape(shape);
		egg->frame = frame;
		egg->quality = quality;
		egg->flags = flags;
		egg->npcnum = npcnum;
		egg->mapnum = mapnum;
		egg->extendedflags = extendedflags;
		if (objid) egg->assignObjId();
		return egg;
	}

	case ShapeInfo::SF_TELEPORTEGG:
	{
		TeleportEgg* egg = new TeleportEgg();
		egg->setShape(shape);
		egg->frame = frame;
		egg->quality = quality;
		egg->flags = flags;
		egg->npcnum = npcnum;
		egg->mapnum = mapnum;
		egg->extendedflags = extendedflags;
		if (objid) egg->assignObjId();
		return egg;
	}

	default:
		return 0;
	}
}



Actor* ItemFactory::createActor(uint32 shape, uint32 frame, uint16 quality,
								uint16 flags, uint16 npcnum, uint16 mapnum,
								uint32 extendedflags, bool objid)
{
	// this function should probably differentiate between the Avatar,
	// NPCs, monsters?

/*
    // This makes it rather hard to create new NPCs...
	if (npcnum == 0) // or do monsters have npcnum 0? we'll see...
		return 0;
*/
	// New actor, no lerping
	extendedflags |= Item::EXT_LERP_NOPREV;

	if (npcnum == 1) {
		// Main Actor

		MainActor* actor = new MainActor();
		actor->setShape(shape);
		actor->frame = frame;
		actor->quality = quality;
		actor->flags = flags;
		actor->npcnum = npcnum;
		actor->mapnum = mapnum;
		actor->objid = 1;
		actor->extendedflags = extendedflags;
		return actor;
	}

	// 'normal' Actor/NPC
	Actor* actor = new Actor();
	actor->setShape(shape);
	actor->frame = frame;
	actor->quality = quality;
	actor->flags = flags;
	actor->npcnum = npcnum;
	actor->mapnum = mapnum;
	if (npcnum != 0) {
		actor->objid = static_cast<uint16>(npcnum);
	} else if (objid) {
		actor->assignObjId();
	}
	actor->extendedflags = extendedflags;

	return actor;
}
