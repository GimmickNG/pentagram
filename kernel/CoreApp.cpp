/*
Copyright (C) 2004-2006 The Pentagram team

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

#include "CoreApp.h"

#include "FileSystem.h"
#include "ConfigFileManager.h"
#include "SettingManager.h"

#include "IDataSource.h"
#include "Args.h"
#include "GameInfo.h"
#include "GameDetector.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using std::string;

static void ToLower(std::string& str);


// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(CoreApp);

CoreApp* CoreApp::application = 0;

CoreApp::CoreApp(int argc_, const char* const* argv_)
	: isRunning(false), gameinfo(0), filesystem(0),
	  configfileman(0), settingman(0), argc(argc_),
	  argv(argv_), oHelp(false), oQuiet(false), oVQuiet(false)
{
	assert(application == 0);
	application = this;
}

CoreApp::~CoreApp()
{
	std::map<Pentagram::istring, GameInfo*>::iterator i;
	for (i = games.begin(); i != games.end(); ++i)
		delete i->second;

	games.clear();

	FORGET_OBJECT(filesystem);
	FORGET_OBJECT(settingman);
	FORGET_OBJECT(configfileman);
	FORGET_OBJECT(gameinfo);

	application = 0;
}

void CoreApp::startup()
{
	DeclareArgs(); // Note: this is virtual

	ParseArgs(argc, argv);

	// if we're spitting out help, we probably want to avoid having the
	// other cruft dumped too...
	if(oHelp) { oQuiet=oVQuiet=true; }
	if(oQuiet)
		con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO &
											~MM_MINOR_WARN));
	if(oVQuiet)
		con.setMsgMask(static_cast<MsgMask>(MM_ALL & ~MM_INFO & ~MM_MINOR_WARN
											& ~MM_MAJOR_WARN & ~MM_MINOR_ERR));

	if (oHelp) {
		helpMe(); // Note: this is virtual
		exit(0);
	}


	sysInit();

	setupVirtualPaths(); // setup @home, @data
	loadConfig(); // load config files
}

void CoreApp::DeclareArgs()
{
	parameters.declare("--game",	&oGamename,	"");
	parameters.declare("-h",		&oHelp, 	true);
	parameters.declare("--help",	&oHelp,		true);
	parameters.declare("-q", 		&oQuiet,	true);
	parameters.declare("-qq",		&oVQuiet,	true);	
}

void CoreApp::sysInit()
{
	gameinfo = 0;

	filesystem = new FileSystem;

	configfileman = new ConfigFileManager();
	settingman = new SettingManager();
	settingman->setDomainName(SettingManager::DOM_GLOBAL, "pentagram");
	settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);

}

void CoreApp::setupVirtualPaths()
{
	// setup the 'base' virtual paths:
	// @home - $HOME/.pentagram/ - for config files, saves,... (OS dependant)
	// @data - /usr/share/pentagram/ - for config files, data,.. (OS dependant)
	//       NB: @data can be overwritten by config files
	//       this should be a default set by configure (or other build systems)

	bool ok;

	const std::string &home = FileSystem::getHomePath();

	ok = filesystem->AddVirtualPath("@home", home, true);
	if (!ok) {
		pout << "Error opening default home directory: " << home << std::endl;
	} else {
		pout << "Default home path: " << home << std::endl;
	}

	std::string data;
#ifdef DATA_PATH
	data = DATA_PATH;
#elif defined(MACOSX)
    data = macosxResourcePath();
#else
	data = "data";
#endif
	ok = filesystem->AddVirtualPath("@data", data);
	if (!ok) {
#ifndef BUILTIN_DATA
		pout << "Error opening default data directory: " << data << std::endl;
		pout << "Trying custom data path specified in configuration file."
			 << std::endl;
#endif
	} else {
		pout << "Default data path: " << data << std::endl;
	}
}

// load configuration files
void CoreApp::loadConfig()
{
	pout << "Loading configuration files:" << std::endl;

	bool dataconf, homeconf;

	// system-wide config, read-only
	dataconf = settingman->readConfigFile("@data/pentagram.ini", true);

	// user config
	homeconf = settingman->readConfigFile("@home/pentagram.ini");

	if (!homeconf && !dataconf) {
		pout << "No configuration files found." << std::endl;
	} else {

		if (dataconf)
			pout << "@data/pentagram.ini" << std::endl;
		if (homeconf)
			pout << "@home/pentagram.ini" << std::endl;
	}

	//  load pentagram specific data path
	std::string data;
	if (settingman->get("data", data, SettingManager::DOM_GLOBAL)) {
		pout << "Setting custom data path: " << data << std::endl;
		bool ok = filesystem->AddVirtualPath("@data", data);
		if (!ok) {
			perr << "Error opening data directory." << std::endl;
		}
	}
}

void CoreApp::setupGameList()
{
	std::vector<Pentagram::istring> gamelist;
	gamelist = settingman->listGames();
	con.Print(MM_INFO, "Scanning config file for games:\n");
	std::vector<Pentagram::istring>::iterator iter;
	Pentagram::istring gamename;

	for (iter = gamelist.begin(); iter != gamelist.end(); ++iter) {
		Pentagram::istring game = *iter;
		GameInfo* info = new GameInfo;
		bool detected = getGameInfo(game, info);

		// output detected game info
		con.Printf(MM_INFO, "%s: ", game.c_str());
		if (detected) {
			// add game to games map
			games[game] = info;

			std::string details = info->getPrintDetails();
			con.Print(MM_INFO, details.c_str());
		} else {
			con.Print(MM_INFO, "unknown, skipping");
		}
		con.Print(MM_INFO, "\n");
	}
}

GameInfo* CoreApp::getDefaultGame()
{
	Pentagram::istring gamename;

	std::string defaultgame;
	bool defaultset = settingman->get("defaultgame", defaultgame,
									  SettingManager::DOM_GLOBAL);

	if (oGamename != "") {
		// game specified on commandline
		gamename = oGamename;
	} else if (defaultset) {
		// default game specified in config file
		gamename = defaultgame;
	} else if (games.size() == 2) {// TODO - Do this in a better method
		// only one game in config file, so pick that
		std::map<Pentagram::istring, GameInfo*>::iterator i;
		for (i = games.begin(); i != games.end(); ++i) {
			if (i->second->name != "pentagram")
				gamename = i->second->name;
		}
	} else if (games.size() == 1) {
		perr << "----------------------------------------" << std::endl
			 << "No games set up in configuration. " << std::endl
			 << "Please read the README for instructions." << std::endl
			 << "----------------------------------------" << std::endl;
		// FIXME - report more useful error message
		return 0;
	} else {		
		perr << "Multiple games found in configuration, but no default "
			 << "game is selected." << std::endl
			 << "Either start Pentagram with the \"--game <gamename>\","
			 << std::endl
			 << "or set pentagram/defaultgame in pentagram.ini"
			 << std::endl;	// FIXME - report more useful error message
		return 0;
	}

	pout << "Default game: " << gamename << std::endl;

	GameInfo* info = getGameInfo(gamename);

	if (!info) {
		perr << "Game \"" << gamename << "\" not found." << std::endl;
	}

	// We've got a default game name, doesn't mean it will work though
	return info;
}

bool CoreApp::setupGame(GameInfo* info)
{
	if (!info) return false;
	assert(info->name != "");

	gameinfo = info;

	pout << "Selected game: " << info->name << std::endl;
	pout << info->getPrintDetails() << std::endl;

	setupGamePaths(info);

	return info->name != "pentagram";
}

void CoreApp::killGame()
{
	// Save the settings!
	pout << "Saving settings" << std::endl;
	settingman->write();

	filesystem->RemoveVirtualPath("@game");
	filesystem->RemoveVirtualPath("@work");
	filesystem->RemoveVirtualPath("@save");

	configfileman->clearRoot("bindings");
	configfileman->clearRoot("language");
	configfileman->clearRoot("weapons");
	configfileman->clearRoot("armour");
	configfileman->clearRoot("monsters");
	configfileman->clearRoot("game");
	settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);

	gameinfo = 0;
}


bool CoreApp::getGameInfo(Pentagram::istring& game, GameInfo* ginfo)
{
	// first try getting the information from the config file
	// if that fails, try to autodetect it

	ginfo->name = game;
	ginfo->type = GameInfo::GAME_UNKNOWN;
	ginfo->version = 0;
	ginfo->language = GameInfo::GAMELANG_UNKNOWN;

	Pentagram::istring gamekey = "settings/";
	gamekey += game;

	if (game == "pentagram") {
		ginfo->type = GameInfo::GAME_PENTAGRAM_MENU;
		ginfo->language = GameInfo::GAMELANG_ENGLISH;
	}
	else {
		std::string gametype;
		if (!configfileman->get(gamekey+"/type", gametype))
			gametype = "unknown";
		ToLower(gametype);

		if (gametype == "u8") {
			ginfo->type = GameInfo::GAME_U8;
		} else if (gametype == "remorse") {
			ginfo->type = GameInfo::GAME_REMORSE;
		}
	}

	std::string version;
	if (!configfileman->get(gamekey+"/version", version))
		version = "unknown";

	std::string language;
	if (!configfileman->get(gamekey+"/language", language))
		language = "unknown";
	ToLower(language);


	//!! TODO: version parsing

	if (language == "english") {
		ginfo->language = GameInfo::GAMELANG_ENGLISH;
	} else if (language == "french") {
		ginfo->language = GameInfo::GAMELANG_FRENCH;
	} else if (language == "german") {
		ginfo->language = GameInfo::GAMELANG_GERMAN;
	} else if (language == "spanish") {
		ginfo->language = GameInfo::GAMELANG_SPANISH;
	} else if (language == "japanese") {
		ginfo->language = GameInfo::GAMELANG_JAPANESE;
	}

	if (ginfo->type == GameInfo::GAME_UNKNOWN ||
		/* ginfo->version == 0 || */
		ginfo->language == GameInfo::GAMELANG_UNKNOWN)
	{
		std::string path;
		if (!configfileman->get(gamekey+"/path", path)) return false;

		return GameDetector::detect(path, ginfo);
	}

	if (ginfo->type == GameInfo::GAME_UNKNOWN) {
		return false;
	}

	return true;
}

void CoreApp::setupGamePaths(GameInfo* ginfo)
{
	if (!ginfo || ginfo->name == "pentagram")
	{
		settingman->setCurrentDomain(SettingManager::DOM_GLOBAL);
		return;
	}

	Pentagram::istring game = ginfo->name;

	settingman->setDomainName(SettingManager::DOM_GAME, game);
	settingman->setCurrentDomain(SettingManager::DOM_GAME);

	// load main game data path
	std::string gpath;
	settingman->get("path", gpath, SettingManager::DOM_GAME);
	filesystem->AddVirtualPath("@game", gpath);
	con.Printf(MM_INFO, "Game Path: %s\n", gpath.c_str());


	// load work path. Default is @home/game-work
	// where 'game' in the above is the specified 'game' loaded
	std::string work;
	if (!settingman->get("work", work, SettingManager::DOM_GAME))
		work = "@home/"+game+"-work";

#if 0
	// force creation if it doesn't exist

	// TODO: I don't like these being created here.
	//       I'd prefer them to be created when needed. (-wjp)

	filesystem->AddVirtualPath("@work", work, true);
	con.Printf(MM_INFO, "U8 Workdir: %s\n", work.c_str()); //!!FIXME (u8)

	// make sure we've got a minimal sane filesystem under there...
	filesystem->MkDir("@work/usecode");
	filesystem->MkDir("@work/usecode/obj");
	filesystem->MkDir("@work/usecode/src");
	filesystem->MkDir("@work/usecode/asm");
#endif

	// load savegame path. Default is @home/game-save
	std::string save;
	if (!settingman->get("save", save, SettingManager::DOM_GAME))
		save = "@home/"+game+"-save";

	// force creation if it doesn't exist
	filesystem->AddVirtualPath("@save", save, true);
	con.Printf(MM_INFO, "Savegame directory: %s\n", save.c_str());
}

void CoreApp::ParseArgs(const int argc_, const char * const * const argv_)
{
	parameters.process(argc_, argv_);
}

void CoreApp::helpMe()
{
	con.Print("\t-h\t\t- quick help menu (this)\n");
	con.Print("\t-q\t\t- silence general logging messages\n");
	con.Print("\t-qq\t\t- silence general logging messages and\n\t\t\t  non-critical warnings/errors\n");
	con.Print("\t--game {name}\t- select a game\n");
}

GameInfo* CoreApp::getGameInfo(Pentagram::istring game) const
{
	std::map<Pentagram::istring, GameInfo*>::const_iterator i;
	i = games.find(game);

	if (i != games.end())
		return i->second;
	else
		return 0;
}


static void ToLower(std::string& str)
{
	for (unsigned int i = 0; i < str.size(); ++i)
	{
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((str[i] >= 'A') && (str[i] <= 'Z')) str[i] += 32;
#else
		str[i] = static_cast<char>(std::tolower(str[i]));
#endif
	}

}
