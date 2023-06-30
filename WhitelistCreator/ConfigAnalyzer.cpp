// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#include <map>
#include <filesystem>
#include <regex>
#include <set>
#include <string>
#include <sstream>
#include <future>
#include <iostream>
#include <fstream>

#include "ConfigAnalyzer.h"
#include "main.h"

std::map<std::string, ConfigAnalyzer::OrderParams> ConfigAnalyzer::retreiveConfig(std::filesystem::path mapdir, bool multithreading /*= false*/, int dbgPrint /*= 0*/)
{
	debugLevel_ = dbgPrint;
	std::multimap<std::string, OrderParams> outmap;
	for (auto& entry : getBaseClassDefinitions(mapdir))
	{
		if (debugLevel_ > 1) std::wcout << entry.wstring() << std::endl;
		auto strstr = loadCfg(entry);
		loadIncludes(&strstr, mapdir);
		//std::cout << strstr.str();
		outmap.merge(configWalker(&strstr));
	}
	return fuseConfig(outmap, multithreading);
}

std::map<std::string, ConfigAnalyzer::OrderParams> ConfigAnalyzer::fuseConfig(std::multimap<std::string, ConfigAnalyzer::OrderParams> dataMap, bool multithreading /*= false*/)
{
	std::map<std::string, OrderParams> outmap;

	std::vector<std::future<OrderParams>> asyncresults;

	for (auto it = dataMap.begin(); it != dataMap.end();)
	{
		auto key = it->first;
		it = dataMap.upper_bound(key);
		auto range = dataMap.equal_range(key);
		if ((std::ranges::distance(range.first, range.second) > 20) && (multithreading))
		{
			asyncresults.push_back(std::async(std::launch::async, &ConfigAnalyzer::processEntries, this, range));
		}
		else
		{
			auto outResults = processEntries(range);
			outmap.insert(std::make_pair(fnName2Key(outResults.Order), outResults));
		}
	}
	if (multithreading)
	{
		for (auto& entry : asyncresults)
		{
			if (entry.valid())
			{
				entry.wait();
				outmap.insert(std::make_pair(fnName2Key(entry.get().Order), entry.get()));
			}
		}
	}

	return outmap;
}
ConfigAnalyzer::OrderParams ConfigAnalyzer::processEntries(std::pair<std::map<std::string, ConfigAnalyzer::OrderParams>::iterator, std::map<std::string, ConfigAnalyzer::OrderParams>::iterator> range)
{
	std::set<int> jipValue;
	std::set<int> allowedTargets;
	OrderParams outResults;

	for (auto& entry = range.first; entry != range.second; ++entry)
	{
		outResults.Order = entry->second.Order;
		outResults.isfuction = entry->second.isfuction;
		outResults.remoteexecMode = entry->second.remoteexecMode;
		outResults.JipWeak = entry->second.JipWeak;

		jipValue.insert(entry->second.jip);
		allowedTargets.insert(entry->second.allowedtargets);
	};
	if (jipValue.contains(JIP_ALLOWED))
	{
		outResults.jip = JIP_ALLOWED;
	}
	else if (jipValue.contains(JIP_FORBIDDEN))
	{
		outResults.jip = JIP_FORBIDDEN;
	}

	if (allowedTargets.contains(TARGET_ALL))
	{
		outResults.allowedtargets = TARGET_ALL;
	}
	else if (allowedTargets.contains(TARGET_CLIENTS) && (allowedTargets.contains(TARGET_SERVER)))
	{
		outResults.allowedtargets = TARGET_ALL;
	}
	else if (allowedTargets.contains(TARGET_CLIENTS))
	{
		outResults.allowedtargets = TARGET_CLIENTS;
	}
	else if (allowedTargets.contains(TARGET_SERVER))
	{
		outResults.allowedtargets = TARGET_SERVER;
	}

	return outResults;
}

std::set<std::filesystem::path> ConfigAnalyzer::getBaseClassDefinitions(std::filesystem::path mapdir)
{
	std::set<std::filesystem::path> outset;
	std::vector<std::string> extensions = CONFIGFILE_EXTENSIONS;

	for (auto const& entry : std::filesystem::recursive_directory_iterator(mapdir))
	{
		auto pth = entry.path().extension().string();
		if (entry.is_regular_file() && entry.path().has_extension() && caseInsensitiveStrEqual(&pth, &extensions))
		{
			std::ifstream file(entry.path(), std::ios::in);
			if (file.is_open() && file.good())
			{
				bool slcom = false;
				bool mlcom = false;
				for (std::string line; std::getline(file, line, ';'); )
				{
					slcom = remSinglelineComments(&line, slcom);
					mlcom = remMultilineComments(&line, mlcom);

					if (hasBaseClassDefinition(&line))
					{
						outset.insert(entry);
					}
				}
			}
			file.close();
		}
	}
	return outset;
}
bool ConfigAnalyzer::hasBaseClassDefinition(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(class)\\s+(CfgRemoteExec)", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(class)\\s+(CfgRemoteExec)", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 

	std::smatch regexMatch;
	return std::regex_search(*line, regexMatch, regex2find);
}
std::multimap<std::string, ConfigAnalyzer::OrderParams> ConfigAnalyzer::configWalker(std::stringstream* sstr)
{
	auto functions = findSubClassFunctions(&sstr->str());
	auto commands = findSubClassCommands(&sstr->str());

	std::string functionData;
	std::string commandData;

	if (!functions.empty() && !commands.empty())
	{
		if (functions.position() < commands.position())
		{
			functionData = sstr->str().substr(functions.position(), sstr->str().size() - commands.length() - functions.position() - 5);
			commandData = sstr->str().substr(commands.position(), commands.length());
		}
		else
		{
			commandData = sstr->str().substr(commands.position(), sstr->str().size() - functions.length() - commands.position() - 5);
			functionData = sstr->str().substr(functions.position(), functions.length() - 5);
		}
	}
	else if (functions.empty() && !commands.empty())
	{
		commandData = sstr->str().substr(commands.position(), commands.length() - 5);
	}
	else if (!functions.empty() && commands.empty())
	{
		functionData = sstr->str().substr(functions.position(), functions.length() - 5);
	}


	std::multimap<std::string, OrderParams> outmap;

	outmap.merge(parseSection(&functionData, true));
	outmap.merge(parseSection(&commandData, false));

	commandData.erase(0, 5);
	functionData.erase(0, 5);

	outmap.merge(parseSection(&functionData, true));
	outmap.merge(parseSection(&commandData, false));

	//std::cout << std::endl << "FunctionData:" << functionData << std::endl;
	//std::cout << std::endl << "CommandData:" << commandData << std::endl;

	return outmap;
}

std::stringstream ConfigAnalyzer::loadCfg(std::filesystem::path path)
{
	std::ifstream file(path, std::ios::in);
	int bracketsOpen = 0;
	std::stringstream sstr;
	if (file.is_open() && file.good())
	{
		bool slcom = false;
		bool mlcom = false;
		for (std::string line; std::getline(file, line, ';'); )
		{
			slcom = remSinglelineComments(&line, slcom);
			mlcom = remMultilineComments(&line, mlcom);

			auto regexMatch = findClassCfgRemoteExec(&line);
			if ((!regexMatch.empty()) || (bracketsOpen > 0))
			{
				size_t offset = 0;
				if (!regexMatch.empty())
				{
					line = regexMatch.suffix().str();
				}
				while (true)
				{
					if (line.find("{", offset) != std::string::npos)
					{
						bracketsOpen++;
						offset = line.find("{", offset) + 1;
					}
					else if (line.find("}", offset) != std::string::npos)
					{
						bracketsOpen--;
						offset = line.find("}", offset) + 1;
						if (bracketsOpen <= 0)
						{
							line.erase(offset, line.size() - offset);
							break;
						};
					}
					else
					{
						break;
					}
				}
				sstr << line;
			}
		}
	}
	file.close();

	return sstr;
}
void ConfigAnalyzer::loadIncludes(std::stringstream* sstr, std::filesystem::path parentPath, int dbgPrint /*= 0*/)
{
#ifdef _MSC_VER
	std::regex regex2find("(#include)\\s*[\\w\\.:/\"\\\\]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(#include)\\s*[\\w\\.:/\"\\\\]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::vector<std::sregex_iterator> matchiterators;

	std::string tmpstr = sstr->str();
	for (std::sregex_iterator i = std::sregex_iterator(tmpstr.begin(), tmpstr.end(), regex2find);
		i != std::sregex_iterator();
		++i)
	{
		matchiterators.push_back(i);
	}
	for (int64_t i = matchiterators.size() - 1; i >= 0; i--)
	{
		auto entry = matchiterators.at(i);
		size_t len2remove = sizeof("#include");
		std::string relPath = cleanupPath(tmpstr.substr(entry->position() + len2remove, entry->length() - len2remove));
		tmpstr.erase(entry->position(), entry->length());

		std::filesystem::path includepath = parentPath;
		includepath.append(relPath);
		includepath = cleanupPath(includepath.string());
		//std::cout << includepath << std::endl;
		if (debugLevel_ > 1) std::wcout << includepath.wstring() << std::endl;
		std::ifstream file(includepath, std::ios::in);
		if (file.is_open() && file.good())
		{
			size_t offset = entry->position();
			bool inComment = false;
			for (std::string line; std::getline(file, line); )
			{
				remSinglelineComments(&line, false);
				inComment = remMultilineComments(&line, inComment);

				if (!inComment)
				{
					tmpstr.insert(offset, line);
					offset += line.size();
				}
			}
		}
		file.close();
	}
	std::stringstream sstrout;
	sstrout << tmpstr;
	sstr->operator=(std::move(sstrout));
}

std::multimap<std::string, ConfigAnalyzer::OrderParams> ConfigAnalyzer::parseSection(std::string* data, bool isfunction /*= false*/)
{
#ifdef _MSC_VER
	std::regex regex2find("(class\\s+\\w+)[\\s\\w{=;]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(class\\s+\\w+)[\\s\\w{=;]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::multimap<std::string, OrderParams> outmap;
	OrderParams outdata;
	outdata.JipWeak = getJip(data);
	outdata.remoteexecMode = getMode(data);

	std::vector<std::string> datalist;

	for (std::sregex_iterator i = std::sregex_iterator(data->begin(), data->end(), regex2find);
		i != std::sregex_iterator();
		++i)
	{
		datalist.push_back(i->str());
	}
	for (auto& entry : datalist)
	{
		entry.erase(0, 5);
		removeWSLB(&entry);
		entry.append("}");
		//std::cout << entry << std::endl;
		outdata.Order = getFunctionname(&entry);
		outdata.allowedtargets = getAllowedTargets(&entry);
		outdata.jip = getJip(&entry);
		outdata.isfuction = isfunction;
		outmap.insert(std::make_pair(fnName2Key(outdata.Order), outdata));
	}
	return outmap;
}

std::smatch ConfigAnalyzer::findClassCfgRemoteExec(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(class)\\s+(CfgRemoteExec)", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(class)\\s+(CfgRemoteExec)", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch;
}
std::smatch ConfigAnalyzer::findSubClassFunctions(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(class\\s+Functions)[\\s\\w{=;}]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(class\\s+Functions)[\\s\\w{=;}]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch;
}
std::smatch ConfigAnalyzer::findSubClassCommands(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(class\\s+Commands)[\\s\\w{=;}]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(class\\s+Commands)[\\s\\w{=;}]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch;
}

std::string ConfigAnalyzer::getFunctionname(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("^[\\w]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("^[\\w]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch.str();
}
int ConfigAnalyzer::getAllowedTargets(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("allowedtargets[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("allowedtargets[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	auto numfirst = regexMatch.str().find_first_of("0123456789");
	auto numlast = regexMatch.str().find_last_of("0123456789");
	if ((numfirst != std::string::npos) && (numlast != std::string::npos) && (numfirst <= numlast))
	{
		return std::stoi(regexMatch.str().substr(numfirst, (numfirst == numlast) ? (1) : (numlast - numfirst)));
	}
	return -1;
}
int ConfigAnalyzer::getJip(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("jip[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("jip[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	auto numfirst = regexMatch.str().find_first_of("0123456789");
	auto numlast = regexMatch.str().find_last_of("0123456789");
	if ((numfirst != std::string::npos) && (numlast != std::string::npos) && (numfirst <= numlast))
	{
		return std::stoi(regexMatch.str().substr(numfirst, (numfirst == numlast) ? (1) : (numlast - numfirst)));
	}
	return -1;
}
int ConfigAnalyzer::getMode(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("mode[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("mode[\\s=\\d]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	auto numfirst = regexMatch.str().find_first_of("0123456789");
	auto numlast = regexMatch.str().find_last_of("0123456789");
	if ((numfirst != std::string::npos) && (numlast != std::string::npos) && (numfirst <= numlast))
	{
		return std::stoi(regexMatch.str().substr(numfirst, (numfirst == numlast) ? (1) : (numlast - numfirst)));
	}
	return -1;
}
