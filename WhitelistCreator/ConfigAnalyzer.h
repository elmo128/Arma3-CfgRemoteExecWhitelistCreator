// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#pragma once

#include <set>

#include "Helper.h"

class ConfigAnalyzer : Helper
{
public:
	ConfigAnalyzer() :debugLevel_(0) {};
	class OrderParams
	{
	public:
		void clear()
		{
			isfuction = false;
			Order.clear();
			allowedtargets = -1;
			jip = -1;
			remoteexecMode = -1;
			JipWeak = -1;
		}
		OrderParams() : isfuction(false), Order(""), allowedtargets(-1), jip(-1), remoteexecMode(-1), JipWeak(-1) {}
		bool isfuction;
		std::string Order;
		int allowedtargets;
		int jip;
		int remoteexecMode;
		int JipWeak;
	};
	std::map<std::string, OrderParams> retreiveConfig(std::filesystem::path mapdir, bool multithreading = false, int dbgPrint = 0);
	void setDebugLevel(int level) { debugLevel_ = level; };
private:
	int debugLevel_;

	std::map<std::string, OrderParams> fuseConfig(std::multimap<std::string, OrderParams> dataMap, bool multithreading = false);
	OrderParams processEntries(std::pair<std::map<std::string, ConfigAnalyzer::OrderParams>::iterator, std::map<std::string, ConfigAnalyzer::OrderParams>::iterator> range);

	std::set<std::filesystem::path> getBaseClassDefinitions(std::filesystem::path mapdir);
	bool hasBaseClassDefinition(std::string* line);
	std::multimap<std::string, OrderParams> configWalker(std::stringstream* sstr);

	std::stringstream loadCfg(std::filesystem::path path);
	void loadIncludes(std::stringstream* sstr, std::filesystem::path parentPath, int dbgPrint = 0);

	std::multimap<std::string, OrderParams> parseSection(std::string* data, bool isfunction = false);

	std::smatch findClassCfgRemoteExec(std::string* line);
	std::smatch findSubClassFunctions(std::string* line);
	std::smatch findSubClassCommands(std::string* line);

	std::string getFunctionname(std::string* line);
	int getAllowedTargets(std::string* line);
	int getJip(std::string* line);
	int getMode(std::string* line);
};
