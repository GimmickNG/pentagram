/*
 *  Copyright (C) 2004 The Pentagram Team
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
#include "TreasureLoader.h"

#include "ConfigFileManager.h"

TreasureLoader::TreasureLoader()
{

}

TreasureLoader::~TreasureLoader()
{

}

void TreasureLoader::loadDefaults()
{
	ConfigFileManager* config = ConfigFileManager::get_instance();
	std::map<Pentagram::istring,std::string> lootkeyvals;

	// load default treasure types
	lootkeyvals = config->listKeyValues("game/treasure");
	std::map<Pentagram::istring,std::string>::iterator defaultiter;
	for (defaultiter = lootkeyvals.begin();
		 defaultiter != lootkeyvals.end(); ++defaultiter)
	{
		TreasureInfo ti;
		bool ok = internalParse(defaultiter->second, ti, true);
		if (ok) {
			defaultTreasure[defaultiter->first] = ti;
		} else {
			perr << "Failed to parse treasure type '" << defaultiter->first
				 << "': " << defaultiter->second << std::endl;
		}
	}

}

bool TreasureLoader::parse(std::string desc,
						   std::vector<TreasureInfo>& treasure)
{
	treasure.clear();

	TreasureInfo ti;
	std::string::size_type pos;
	while (!desc.empty()) {
		pos = desc.find(';');
		std::string item = desc.substr(0, pos);
//		pout << "parse: item=" << item << std::endl;
		if (internalParse(desc.substr(0, pos), ti, false)) {
			treasure.push_back(ti);
		} else {
			return false;
		}

		if (pos != std::string::npos) pos++;
		desc.erase(0, pos);
	}

	return true;
}

bool TreasureLoader::internalParse(std::string desc, TreasureInfo& ti,
								   bool loadingDefault)
{
	// TODO: be a bit more liberal with the separation into key=value pairs

	ti.special = "";
	ti.chance = 1;
	ti.shapes.clear();
	ti.frames.clear();
	ti.mincount = ti.maxcount = 1;

	bool loadedDefault = false;

	std::string::size_type pos;
	while (!desc.empty()) {
		pos = desc.find(' ');
		std::string item = desc.substr(0, pos);
//		pout << "internalParse: item=" << item << std::endl;

		std::string::size_type itempos = item.find('=');
		if (itempos == std::string::npos ||
			itempos == 0 || itempos+1 >= item.size())
		{
			return false;
		}
		std::string key = item.substr(0, itempos);
		std::string val = item.substr(itempos+1);

		if (key == "shape") {
			if (!parseUInt32Vector(val, ti.shapes))
				return false;
		} else if (key == "frame") {
			if (!parseUInt32Vector(val, ti.frames))
				return false;
		} else if (key == "count") {
			if (!parseUIntRange(val, ti.mincount, ti.maxcount)) {
				int x;
				if (!parseInt(val, x))
					return false;
				ti.mincount = ti.maxcount = x;
			}
		} else if (key == "chance") {
			if (!parseDouble(val, ti.chance))
				return false;
		} else if (key == "special" && loadingDefault) {
			ti.special = val;
		} else if (key == "type" && !loadingDefault) {
			std::map<Pentagram::istring, TreasureInfo>::iterator iter;
			iter = defaultTreasure.find(val);
			if (iter != defaultTreasure.end())
				ti = iter->second;
			else
				return false;
			loadedDefault = true;
		} else if (key == "mult" && !loadingDefault) {
			if (!loadedDefault) return false;
			unsigned int minmult, maxmult;
			if (!parseUIntRange(val, minmult, maxmult)) {
				int x;
				if (!parseInt(val, x))
					return false;
				minmult = maxmult = x;
			}
			ti.mincount *= minmult;
			ti.maxcount *= maxmult;
		} else {
			return false;
		}

		if (pos != std::string::npos) pos++;
		desc.erase(0, pos);
	}

	return true;
}

bool TreasureLoader::parseUInt32Vector(std::string val,
									   std::vector<uint32>& vec)
{
	vec.clear();

	std::string::size_type pos;
	while (!val.empty()) {
		pos = val.find(',');
		std::string item = val.substr(0, pos);

		std::string::size_type itempos = val.find('-');
		if (itempos != std::string::npos) {
			unsigned int min, max;
			if (!parseUIntRange(item, min, max))
				return false;
			for (unsigned int i = min; i <= max; ++i)
				vec.push_back(i);
		} else {
			int x;
			if (!parseInt(item, x))
				return false;
			vec.push_back(x);
		}
		
		if (pos != std::string::npos) pos++;
		val.erase(0, pos);
	}

	return true;
}

bool TreasureLoader::parseUIntRange(std::string val,
								   unsigned int& min, unsigned int& max)
{
	std::string::size_type pos = val.find('-');
	if (pos == 0 || pos == std::string::npos || pos+1 >= val.size())
		return false;
	int t1, t2;
	bool ok = true;
	ok &= parseInt(val.substr(0, pos), t1);
	ok &= parseInt(val.substr(pos+1), t2);
	if (ok) {
		min = t1;
		max = t2;
	}
	return ok;
}

bool TreasureLoader::parseDouble(std::string val, double& d)
{
	// TODO: error checking
	d = std::strtod(val.c_str(), 0);
	return true;
}

bool TreasureLoader::parseInt(std::string val, int& i)
{
	// TODO: error checking
	i = std::strtol(val.c_str(), 0, 0);
	return true;
}
