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

#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <vector>
#include "FrameID.h"

class RawArchive;
class MainShapeArchive;
class FontShapeArchive;
class GumpShapeArchive;
class ShapeArchive;
class Usecode;
class MapGlob;
class Shape;
class MusicFlex;
class WpnOvlayDat;
class ShapeFrame;
class SoundFlex;
class SpeechFlex;
struct GameInfo;

class GameData
{
public:
	GameData(GameInfo *gameinfo);
	~GameData();

	static GameData* get_instance() { return gamedata; }

	void loadU8Data();
	void loadRemorseData();
	void setupFontOverrides();

	Usecode* getMainUsecode() const { return mainusecode; }
	MainShapeArchive* getMainShapes() const { return mainshapes; }
	RawArchive* getFixed() const { return fixed; }
	MapGlob* getGlob(uint32 glob) const;
	FontShapeArchive* getFonts() const { return fonts; }
	GumpShapeArchive* getGumps() const { return gumps; }
	Shape* getMouse() const { return mouse; }
	MusicFlex* getMusic() const { return music; }
	WpnOvlayDat* getWeaponOverlay() const { return weaponoverlay; }
	SoundFlex* getSoundFlex() const { return soundflex; }
	SpeechFlex* getSpeechFlex(uint32 shapenum);

	ShapeArchive* getShapeFlex(uint16 flexId) const;
	Shape* getShape(FrameID frameid) const;
	ShapeFrame* getFrame(FrameID frameid) const;

	std::string translate(std::string text);
	FrameID translate(FrameID frame);

	enum ShapeFlexId {
		OTHER		= 0,
		MAINSHAPES	= 1,
		GUMPS		= 2
	};
private:
	void loadTranslation();
	void setupTTFOverrides(const char* configkey, bool SJIS);
	void setupJPOverrides();

	RawArchive* fixed;
	MainShapeArchive* mainshapes;
	Usecode* mainusecode;
	std::vector<MapGlob*> globs;
	FontShapeArchive* fonts;
	GumpShapeArchive* gumps;
	Shape* mouse;
	MusicFlex* music;
	WpnOvlayDat* weaponoverlay;

	SoundFlex* soundflex;
	std::vector<SpeechFlex**> speech;
	GameInfo *gameinfo;

	static GameData* gamedata;
};

#define _TL_(x) (GameData::get_instance()->translate(x))
#define _TL_SHP_(x) (GameData::get_instance()->translate(x))

#endif
