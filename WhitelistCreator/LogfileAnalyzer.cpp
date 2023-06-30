// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#include <map>
#include <filesystem>
#include <string>
#include <vector>
#include <future>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>

#include "LogfileAnalyzer.h"
#include "main.h"

std::map<std::string, LogfileAnalyzer::exeResults> LogfileAnalyzer::ProcessLogfilesP(std::filesystem::path paths, bool mulicore /*= true*/, int dbgPrint /*= 0*/)
{
	debugLevel_ = dbgPrint;
	return ProcessLogfiles(paths, mulicore, dbgPrint);
}

std::multimap<std::string, LogfileAnalyzer::exeResults> LogfileAnalyzer::FileAnalysis(std::vector<std::filesystem::path> paths, bool multithreading/*= false*/ )
{
	std::multimap<std::string, exeResults> resultmap;
	if (multithreading)
	{
		std::vector<std::future <std::multimap<std::string, exeResults> > > asyncresults;

		for (const auto& path : paths)
		{
			asyncresults.push_back(std::async(std::launch::async, &LogfileAnalyzer::analyzeLogfile, this, path));
		}

		for (auto& entry : asyncresults)
		{
			if (entry.valid())
			{
				entry.wait();
				resultmap.merge(entry.get());
			}
		}
	}
	else
	{
		for (const auto& path : paths)
		{
			resultmap.merge(analyzeLogfile(path));
		}
	}
	return resultmap;
}
std::multimap<std::string, LogfileAnalyzer::exeResults> LogfileAnalyzer::analyzeLogfile(std::filesystem::path path)
{
	if (debugLevel_ > 1) std::wcout << path.wstring() << std::endl;
	std::multimap<std::string, exeResults> results;
	std::ifstream file(path, std::ios::in);
	if (file.is_open() && file.good())
	{
		for (std::string line; std::getline(file, line); )
		{
			if (lineisEntry(&line))
			{
				auto result = parseLine(&line);
				if (result.first.compare("\"#~") != CPEQUAL)
				{
					results.insert(result);
				}
			};
		}
	}
	file.close();
	return results;
}
bool LogfileAnalyzer::lineisEntry(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(\"A3WLC\\>\\[)([a-zA-Z0-9_]+,-?\\d+,-?\\d+,(true|false))(\\]\")", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(\"A3WLC\\>\\[)([a-zA-Z0-9_]+,-?\\d+,-?\\d+,(true|false))(\\]\")", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	return std::regex_search(*line, regex2find);
}
std::pair<std::string, LogfileAnalyzer::exeResults> LogfileAnalyzer::parseLine(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("([a-zA-Z0-9_]+,-?\\d+,-?\\d+,(true|false))", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("([a-zA-Z0-9_]+,-?\\d+,-?\\d+,(true|false))", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::smatch regexMatch;

	if ((std::regex_search(*line, regexMatch, regex2find))
		&& (regexMatch.suffix().str().find("]\"") != std::string::npos)
		&& (regexMatch.prefix().str().find("\"A3WLC>[") != std::string::npos))
	{
		std::stringstream strstr(regexMatch.str());
		exeResults elements;
		int index = 0;
		for (std::string entry; std::getline(strstr, entry, ','); )
		{
			switch (index)
			{
			case 0:
				elements.setFunction(entry);
				break;
			case 1:
				break;
			case 2:
				elements.setTargetsAllowed(parseValue(&entry));
				break;
			case 3:
				elements.setJipAllowed(parseValue(&entry));
				break;
			}
			index++;
		}
		if ((!elements.getFunction().empty()) && (index >= 3))
		{
			return std::make_pair(fnName2Key(elements.getFunction()), elements);
		}
	}
	return std::make_pair(fnName2Key(""), exeResults());
}
std::map<std::string, LogfileAnalyzer::exeResults> LogfileAnalyzer::processLogEntries(std::multimap<std::string, exeResults> dataMap, bool multithreading /*= false*/)
{
	std::map<std::string, exeResults> outmap;
	std::vector<std::future<exeResults>> asyncresults;

	for (auto it = dataMap.begin(); it != dataMap.end();)
	{
		auto key = it->first;
		it = dataMap.upper_bound(key);
		auto range = dataMap.equal_range(key);
		if ((std::ranges::distance(range.first, range.second) > 200) && (multithreading))
		{
			asyncresults.push_back(std::async(std::launch::async, &LogfileAnalyzer::processEntries, this, range));
		}
		else
		{
			auto outResults = processEntries(range);
			outmap.insert(std::make_pair(fnName2Key(outResults.getFunction()), outResults));
		}
	}
	if (multithreading)
	{
		for (auto& entry : asyncresults)
		{
			if (entry.valid())
			{
				entry.wait();
				outmap.insert(std::make_pair(fnName2Key(entry.get().getFunction()), entry.get()));
			}
		}
	}

	return outmap;
}
LogfileAnalyzer::exeResults LogfileAnalyzer::processEntries(std::pair<std::map<std::string, LogfileAnalyzer::exeResults>::iterator, std::map<std::string, LogfileAnalyzer::exeResults>::iterator> range)
{
	std::set<int> processingTargets;
	std::set<int> jipExecuted;
	exeResults outResults;
	int clientOwner = TARGET_ALL;
	for (auto& entry = range.first; entry != range.second; ++entry)
	{
		outResults.setFunction(entry->second.getFunction());

		//auto remoteExecutedOwner = entry->second.getTargetsAllowed();
		clientOwner = entry->second.getTargetsAllowed();
		auto wasJIPexecuted = entry->second.getJipAllowed();

		if (clientOwner == TARGET_SERVER)
		{
			processingTargets.insert(TARGET_SERVER);
		}
		else if (clientOwner != TARGET_SERVER)
		{
			processingTargets.insert(TARGET_CLIENTS);
		}
		jipExecuted.insert(wasJIPexecuted);
	};

	if (processingTargets.contains(TARGET_SERVER) && processingTargets.contains(TARGET_CLIENTS))
	{
		outResults.setTargetsAllowed(TARGET_ALL);
	}
	else if (processingTargets.contains(TARGET_SERVER))
	{
		outResults.setTargetsAllowed(TARGET_SERVER);
	}
	else if (processingTargets.contains(TARGET_CLIENTS))
	{
		outResults.setTargetsAllowed(TARGET_CLIENTS);
	}

	if (jipExecuted.contains(JIP_ALLOWED))
	{
		outResults.setJipAllowed(JIP_ALLOWED);
	}
	else
	{
		outResults.setJipAllowed(JIP_FORBIDDEN);
	}
	return outResults;
}
int LogfileAnalyzer::parseValue(std::string* element)
{
	std::string tr = "true";
	std::string fs = "false";
	if (caseInsensitiveStrEqual(&tr, element))
	{
		return 1;
	}
	else if (caseInsensitiveStrEqual(&fs, element))
	{
		return 0;
	}
	else
	{
		try
		{
			return std::stoi(*element);
		}
		catch (std::exception ex)
		{
			return 1;
		}
	}
}
