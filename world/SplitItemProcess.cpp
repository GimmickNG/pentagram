/*
Copyright (C) 2004 The Pentagram team

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

#include "SplitItemProcess.h"
#include "World.h"
#include "Item.h"
#include "ShapeInfo.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(SplitItemProcess,Process);

SplitItemProcess::SplitItemProcess() : Process()
{

}

SplitItemProcess::SplitItemProcess(Item* original, Item* target_)
{
	assert(original);
	assert(target_);

	assert(original->getShapeInfo()->hasQuantity());
	assert(target_->getShapeInfo()->hasQuantity());

	item_num = original->getObjId();
	target = target_->getObjId();

	// type = TODO
}

bool SplitItemProcess::run(const uint32 framenum)
{
	Item* original = World::get_instance()->getItem(item_num);
	Item* targetitem = World::get_instance()->getItem(target);

	assert(original);
	assert(targetitem);
	assert(original->getShapeInfo()->hasQuantity());
	assert(targetitem->getShapeInfo()->hasQuantity());

	uint16 movecount = static_cast<uint16>(result);

	assert(movecount <= original->getQuality());

	uint16 origcount = original->getQuality() - movecount;
	uint16 targetcount = targetitem->getQuality() + movecount;

	pout << "SplitItemProcess splitting: " << movecount << ": "
		 << origcount << "-" << targetcount << std::endl;
		

	if (origcount > 0) {
		original->setQuality(origcount);
		original->callUsecodeEvent_combine();
	} else {
		original->destroy();
		original = 0;
	}

	if (targetcount > 0) {
		targetitem->setQuality(targetcount);
		targetitem->callUsecodeEvent_combine();
	} else {
		targetitem->destroy();
		targetitem = 0;
	}

	result = 0;
	terminate();

	return true;
}

void SplitItemProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write2(target);
}

bool SplitItemProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	target = ids->read2();

	return true;
}