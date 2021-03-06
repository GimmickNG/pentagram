/*
Copyright (C) 2004-2007 The Pentagram team

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

#include "CombatProcess.h"
#include "Actor.h"
#include "CurrentMap.h"
#include "World.h"
#include "UCList.h"
#include "LoopScript.h"
#include "WeaponInfo.h"
#include "AnimationTracker.h"
#include "Kernel.h"
#include "DelayProcess.h"
#include "PathfinderProcess.h"
#include "ShapeInfo.h"
#include "MonsterInfo.h"
#include "getObject.h"
#include "LoiterProcess.h"
#include "AmbushProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CombatProcess,Process);

CombatProcess::CombatProcess() : Process()
{

}

CombatProcess::CombatProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x00F2; // CONSTANT !
	target = 0;
	fixedTarget = 0;
	combatmode = CM_WAITING;
}

void CombatProcess::terminate()
{
	Actor* a = getActor(item_num);
	if (a)
		a->clearActorFlag(Actor::ACT_INCOMBAT);

	Process::terminate();
}

void CombatProcess::run()
{
	// TODO: handle invisible targets.
	// Monsters should attack you only if you are standing directly
	// next to them, or maybe only when you are attacking them.
	// They should not try to approach.

	Actor* a = getActor(item_num);
	if (!(a->getFlags() & Item::FLG_FASTAREA))
		return;

	Actor* t = getActor(target);

	if (!t || !isValidTarget(t)) {
		// no target? try to find one

		target = seekTarget();

		if (!target) {
			waitForTarget();
			return;
		}

		pout << "[COMBAT " << item_num << "] target found: "
			 << target << std::endl;
		combatmode = CM_WAITING;
	}

	int targetdir = getTargetDirection();
	if (a->getDir() != targetdir) {
		turnToDirection(targetdir);
		return;
	}

	if (inAttackRange()) {
		combatmode = CM_ATTACKING;

		pout << "[COMBAT " << item_num << "] target (" << target
			 << ") in range" << std::endl;

		bool hasidle1 = a->hasAnim(Animation::idle1);
		bool hasidle2 = a->hasAnim(Animation::idle2);

		if ((hasidle1 || hasidle2) && (std::rand()%5) == 0) {
			// every once in a while, act threatening instead of attacking
			// TODO: maybe make frequency depend on monster type
			Animation::Sequence idleanim;

			if (!hasidle1) {
				idleanim = Animation::idle2;
			} else if (!hasidle2) {
				idleanim = Animation::idle1;
			} else {
				if (std::rand()%2)
					idleanim = Animation::idle1;
				else
					idleanim = Animation::idle2;
			}
			uint16 idlepid = a->doAnim(idleanim, 8);
			waitFor(idlepid);			
 		} else {

			// attack
			ProcId attackanim = a->doAnim(Animation::attack, 8);

			// wait a while, depending on dexterity, before attacking again
			int dex = a->getDex();
			if (dex < 25) {
				int recoverytime = 3 * (25 - dex);
				Process* waitproc = new DelayProcess(recoverytime);
				ProcId waitpid = Kernel::get_instance()->addProcess(waitproc);
				waitproc->waitFor(attackanim);
				waitFor(waitpid);
			} else {
				waitFor(attackanim);
			}
		}

		return;
	} else if (combatmode != CM_PATHFINDING) {
		// not in range? See if we can get in range

		Process* pfproc = new PathfinderProcess(a, target, true);
		
		waitFor(Kernel::get_instance()->addProcess(pfproc));
		combatmode = CM_PATHFINDING;
		return;
	}

	combatmode = CM_WAITING;
	waitForTarget();
}

ObjId CombatProcess::getTarget()
{
	Actor* t = getActor(target);

	if (!t || !isValidTarget(t))
		target = 0;

	return target;
}

void CombatProcess::setTarget(ObjId newtarget)
{
	if (fixedTarget == 0) {
		fixedTarget = newtarget; // want to prevent seekTarget from changing it
	}

	target = newtarget;
}

bool CombatProcess::isValidTarget(Actor* target)
{
	assert(target);
	Actor* a = getActor(item_num);
	if (!a) return false; // uh oh

	// don't target self
	if (target == a) return false;

	// not in the fastarea
	if (!(target->getFlags() & Item::FLG_FASTAREA)) return false;

	// dead actors don't make good targets
	if (target->isDead()) return false;

	// feign death only works on undead and demons
	if (target->getActorFlags() & Actor::ACT_FEIGNDEATH) {

		if ((a->getDefenseType() & WeaponInfo::DMG_UNDEAD) || 
			(a->getShape() == 96)) return false; // CONSTANT!
	}

	// otherwise, ok
	return true;
}

bool CombatProcess::isEnemy(Actor* target)
{
	assert(target);

	Actor* a = getActor(item_num);
	if (!a) return false; // uh oh

	return ((a->getEnemyAlignment() & target->getAlignment()) != 0);
}

ObjId CombatProcess::seekTarget()
{
	Actor* a = getActor(item_num);
	if (!a) return 0; // uh oh

	if (fixedTarget) {
		Actor* t = getActor(fixedTarget);
		if (t && isValidTarget(t))
			return fixedTarget; // no need to search
	}

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	CurrentMap* cm = World::get_instance()->getCurrentMap();
	cm->areaSearch(&itemlist, script, sizeof(script), a, 768, false);

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Actor* t = getActor(itemlist.getuint16(i));

		if (t && isValidTarget(t) && isEnemy(t)) {
			// found target
			return itemlist.getuint16(i);
		}
	}

	// no suitable targets
	return 0;
}

int CombatProcess::getTargetDirection()
{
	Actor* a = getActor(item_num);
	Actor* t = getActor(target);

	return a->getDirToItemCentre(*t);
}

void CombatProcess::turnToDirection(int direction)
{
	Actor* a = getActor(item_num);
	int curdir = a->getDir();
	int step = 1;
	if ((curdir - direction + 8) % 8 < 4) step = -1;
	Animation::Sequence turnanim = Animation::combatStand;

	ProcId prevpid = 0;
	bool done = false;

	for (int dir = curdir; !done; ) {
		ProcId animpid = a->doAnim(turnanim, dir);

		if (dir == direction) done = true;

		if (prevpid) {
			Process* proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;

		dir = (dir + step + 8) % 8;
	}

	if (prevpid) waitFor(prevpid);	
}

bool CombatProcess::inAttackRange()
{
	Actor* a = getActor(item_num);
	ShapeInfo* shapeinfo = a->getShapeInfo();
	MonsterInfo* mi = 0;
	if (shapeinfo) mi = shapeinfo->monsterinfo;

	if (mi && mi->ranged)
		return true; // ranged attacks (ghost's fireball) always in range

	AnimationTracker tracker;
	if (!tracker.init(a, Animation::attack, a->getDir(), 0))
		return false;

	while (tracker.step()) {
		if (tracker.hitSomething()) break;
	}

	ObjId hit = tracker.hitSomething();
	if (hit == target) return true;

	return false;
}

void CombatProcess::waitForTarget()
{
	Actor* a = getActor(item_num);
	ShapeInfo* shapeinfo = a->getShapeInfo();
	MonsterInfo* mi = 0;
	if (shapeinfo) mi = shapeinfo->monsterinfo;

	if (mi && mi->shifter && a->getMapNum() != 43 && (std::rand()%2) == 0 ) {
		// changelings (except the ones at the U8 endgame pentagram)

		// shift into a tree if nobody is around

		ProcId shift1pid = a->doAnim(static_cast<Animation::Sequence>(20), 8);
		Process* ambushproc = new AmbushProcess(a);
		ProcId ambushpid = Kernel::get_instance()->addProcess(ambushproc);
		ProcId shift2pid = a->doAnim(static_cast<Animation::Sequence>(21), 8);
		Process* shift2proc = Kernel::get_instance()->getProcess(shift2pid);

		ambushproc->waitFor(shift1pid);
		shift2proc->waitFor(ambushpid);
		waitFor(shift2proc);

	} else {
		waitFor(Kernel::get_instance()->addProcess(new LoiterProcess(a, 1)));
	}
}

void CombatProcess::dumpInfo()
{
	Process::dumpInfo();
	pout << "Target: " << target << std::endl;
}

void CombatProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write2(target);
	ods->write2(fixedTarget);
	ods->write1(static_cast<uint8>(combatmode));
}

bool CombatProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	target = ids->read2();
	fixedTarget = ids->read2();
	combatmode = static_cast<CombatMode>(ids->read1());

	return true;
}
