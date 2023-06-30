// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#include <string>
#include <map>
#include <vector>
#include <regex>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "FileAnalyzer.h"
#include "main.h"

std::map<std::string, FileAnalyzer::OrderParams<int, int>> FileAnalyzer::getDataDir(std::filesystem::path path, int dbgPrint /*= false*/)
{
	debugLevel_ = dbgPrint;
	return Interpreter(ParseDir(path));
};
std::map<std::string, FileAnalyzer::OrderParams<int, int>> FileAnalyzer::getDataFile(std::filesystem::path path, int dbgPrint /*= false*/)
{
	debugLevel_ = dbgPrint;
	return Interpreter(parseFile(path));
}
std::map<std::string, FileAnalyzer::OrderParams<int, int>> FileAnalyzer::getDataLine(const std::string* line, std::filesystem::path path /*= ""*/)
{
	return Interpreter(ParseLine(line));
}

std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> FileAnalyzer::ParseDir(std::filesystem::path path)
{
	std::vector<std::string> extensiontypes = SCRIPTFILE_EXTENSIONS;
	std::multimap<std::string, OrderParams<std::string, std::string>> result;
	for (auto const& entry : std::filesystem::recursive_directory_iterator(path))
	{
		auto pth = entry.path().extension().string();
		if (entry.is_regular_file() && entry.path().has_extension() && caseInsensitiveStrEqual(&pth, &extensiontypes))
		{
			result.merge(parseFile(entry));
		}
	}
	return result;
};
std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> FileAnalyzer::parseFile(std::filesystem::path path)
{
	if (debugLevel_ > 1) std::wcout << path.wstring() << std::endl;
	std::ifstream file(path, std::ios::in);
	std::multimap<std::string, OrderParams<std::string, std::string>> result;
	if (file.is_open() && file.good())
	{
		bool slcom = false;
		bool mlcom = false;
		for (std::string line; std::getline(file, line, ';'); )
		{
			slcom = remSinglelineComments(&line, slcom);
			mlcom = remMultilineComments(&line, mlcom);
			result.merge(ParseLine(&line));
		}
	}
	file.close();
	return result;
}
std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> FileAnalyzer::ParseLine(const std::string* line)
{
	int occurrences = 0;

#ifdef _MSC_VER
	std::regex regex2find("(?=remoteexec)[\\w\\s\"\\[,\\]\\-\\:]+", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(?=remoteexec)[\\w\s\"\\[,\\]\\-\\:]+", std::regex::ECMAScript | std::regex::icase | std::regex::multiline);
#endif 
	std::multimap<std::string, OrderParams<std::string, std::string>> result;
	for (std::sregex_iterator i = std::sregex_iterator(line->begin(), line->end(), regex2find);
		i != std::sregex_iterator();
		++i)
	{
		std::string afterRemotexec = i->str();
		removeMWSLB(&afterRemotexec);
		afterRemotexec = findPrimaryArray(&afterRemotexec).str();

		//std::cout << "Array: " << afterRemotexec << std::endl;
		auto dataentry = getArguments(orderWalker(&afterRemotexec));
		result.insert(std::make_pair(fnName2Key(dataentry.Order), dataentry));
	}
	return result;
}
std::vector<std::string> FileAnalyzer::orderWalker(std::string* line)
{
	std::vector<std::string> elements;

	int lastSectionEnd = 0;
	int position = 0;
	int openArrays = 0;
	int openCode = 0;
	int openBrackets = 0;
	bool openString = false;
	bool quitnow = false;

	if (line->starts_with('['))
	{
		lastSectionEnd = 1;
	}

	for (const auto& entry : *line)
	{
		std::string entrys;
		entrys = entry;

		if (entrys.compare("[") == CPEQUAL)
		{
			openArrays++;
		}
		if (entrys.compare("{") == CPEQUAL)
		{
			openCode++;
		}
		if (entrys.compare("(") == CPEQUAL)
		{
			openBrackets++;
		}
		if (entrys.compare("\"") == CPEQUAL)
		{
			openString = !openString;
		}
		else if (entrys.compare("}") == CPEQUAL)
		{
			openCode--;
		}
		else if (entrys.compare(")") == CPEQUAL)
		{
			openBrackets--;
		}
		else if (entrys.compare("]") == CPEQUAL)
		{
			openArrays--;
			if (openArrays <= 0)
				quitnow = true;
		}

		if (((entrys.compare(",") == CPEQUAL)
			&& (openArrays == 1)
			&& (openCode == 0)
			&& (openBrackets == 0)
			&& (openString == false)
			) || (quitnow))
		{
			std::string remoteexecargumentsstr = line->substr(lastSectionEnd, (int64_t)position - lastSectionEnd);
			cleanupString(&remoteexecargumentsstr);
			elements.push_back(remoteexecargumentsstr);
			lastSectionEnd = position;
		}
		if (quitnow)
			break;
		position++;
	}
	return elements;
}
std::smatch FileAnalyzer::findPrimaryArray(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("(\\[)[\\w\\W]*(\\])", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("(\\[)[\\w\\W]*(\\])", std::regex::ECMAScript | std::regex::icase);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch;
}
FileAnalyzer::OrderParams<std::string, std::string> FileAnalyzer::getArguments(std::vector<std::string> elements)
{
	OrderParams<std::string, std::string> data;
	if (elements.size() == 1)
	{
		data.Order = elements.at(0);
		data.isfuction = isfunction(&data.Order);
	}
	else if (elements.size() == 2)
	{
		data.Order = elements.at(0);
		data.isfuction = isfunction(&data.Order);
		data.target = elements.at(1);
	}
	else if (elements.size() == 3)
	{
		data.Order = elements.at(0);
		data.isfuction = isfunction(&data.Order);
		data.target = elements.at(1);
		data.jip = elements.at(2);
	};
	return data;
};
bool FileAnalyzer::isfunction(std::string* order)
{
#ifdef _MSC_VER
	std::regex regex2find("_fnc_", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("_fnc_", std::regex::ECMAScript | std::regex::icase);
#endif 
	std::smatch regexMatch;
	return std::regex_search(*order, regexMatch, regex2find);
}
std::map<std::string, FileAnalyzer::OrderParams<int, int>> FileAnalyzer::Interpreter(std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> map)
{
	std::map<std::string, OrderParams<int, int>> results;

	for (auto it = map.begin(); it != map.end();)
	{
		OrderParams<int, int> result;
		auto key = it->first;
		it = map.upper_bound(key);

		auto range = map.equal_range(key);
		std::vector <OrderParams<std::string, std::string>> valuelist;
		for (auto& entry = range.first; entry != range.second; ++entry)
		{
			valuelist.push_back(entry->second);
			//std::cout << "ENTRY: " << entry->first << ", " << entry->second.Order << ", " << entry->second.target << ", " << entry->second.jip << ", " << entry->second.isfuction << std::endl;
		}
		auto rightsdata = getmaxRequiredRights(&valuelist);
		auto jipdata = getJipFlag(&valuelist);

		result.getfromOtherVariant(rightsdata);
		result.getfromOtherVariant(jipdata);
		results.insert(std::make_pair(fnName2Key(result.Order), result));
	}
	return results;
}
FileAnalyzer::OrderParams<int, std::string> FileAnalyzer::getmaxRequiredRights(std::vector<FileAnalyzer::OrderParams<std::string, std::string>>* data)
{
	std::multimap<int, OrderParams<int, std::string>> workingdata;

	for (auto& entry : *data)
	{
		OrderParams<int, std::string> result;
		result.getfromOtherVariant(entry);

		try
		{
			// handle numbers
			result.target = parseNumber(entry.target);
		}
		catch (std::exception ex)
		{
			// array of all possible types. can be threated, if it only contains numbers. As soon as there are other elements, fallback to max elevation. code around the array is ignored, assuming a list, or select comman.
			if (entry.target.find("[") && entry.target.find("]", entry.target.find("["))) // has an array
			{
				auto targetstr = entry.target;
				auto parsedArray = parseArray(&targetstr);
				for (const auto& entry : parsedArray)
				{
					try
					{
						// handle numbers
						result.target = parseNumber(entry);
					}
					catch (std::exception ex)
					{
						result.target = TARGET_ALL;
					}
				}
			}
			else
			{
				// objects, sides, groups, strings (variable name) => can't be threathed outside of the engine. Fallback to max elevation.
				result.target = TARGET_ALL;
			}

		}
		workingdata.insert(std::make_pair(result.target, result));
	}
	OrderParams<int, std::string> returndata;
	if (workingdata.contains(TARGET_ALL))
	{
		returndata.getfromOtherVariant(workingdata.extract(TARGET_ALL).mapped());
	}
	else if (workingdata.contains(TARGET_CLIENTS) && (workingdata.contains(TARGET_SERVER)))
	{
		returndata.getfromOtherVariant(workingdata.extract(TARGET_CLIENTS).mapped());
		returndata.target = TARGET_ALL;
	}
	else if (workingdata.contains(TARGET_CLIENTS))
	{
		returndata.getfromOtherVariant(workingdata.extract(TARGET_CLIENTS).mapped());
	}
	else if (workingdata.contains(TARGET_SERVER))
	{
		returndata.getfromOtherVariant(workingdata.extract(TARGET_SERVER).mapped());
	}
	else
	{
		returndata.target = TARGET_ALL;
	}
	return returndata;
}
FileAnalyzer::OrderParams<std::string, int> FileAnalyzer::getJipFlag(std::vector<FileAnalyzer::OrderParams<std::string, std::string>>* data)
{
	OrderParams<std::string, int> returnvalue;
	for (auto& entry : *data)
	{
		removeWSLB(&entry.jip);
		if (!((entry.jip.compare("false") == CPEQUAL)
			|| (entry.jip.compare("\"\"") == CPEQUAL)
			|| (entry.jip.empty())))
		{
			returnvalue.getfromOtherVariant(entry);
			returnvalue.jip = JIP_ALLOWED;
			break;
		}
		else if (returnvalue.jip != JIP_FORBIDDEN)
		{
			returnvalue.getfromOtherVariant(entry);
			returnvalue.jip = JIP_FORBIDDEN;
		}
	}
	return returnvalue;
}
int FileAnalyzer::parseNumber(std::string target)
{
	int allowedTargets = TARGET_ALL;
	// handle numbers
	int number = std::stoi(target);
	if ((number == 0) || (number < -2) || (number == -1) || (number == 1)) // according to https://community.bistudio.com/wiki/Multiplayer_Scripting#Machine_network_ID partly not implemented, but included to be future proof.                
	{
		allowedTargets = TARGET_ALL;
	}
	else if ((number == -2) || (number > 2))
	{
		allowedTargets = TARGET_CLIENTS;
	}
	else if (number == 2)
	{
		allowedTargets = TARGET_SERVER;
	}
	return allowedTargets;
}
std::vector<std::string> FileAnalyzer::parseArray(const std::string* line)
{
	std::string argumentstring = line->substr(0, getfinalCloser(line, "[", "]"));
	return orderWalker(&argumentstring);
}
int FileAnalyzer::getfinalCloser(const std::string* line, const std::string Opener, const std::string Closer)
{
	int position = 0;
	int openArrays = 0;
	for (const auto& entry : *line)
	{
		std::string entrys;
		entrys = entry;

		if (entrys.compare(Opener) == CPEQUAL)
		{
			openArrays++;
		}
		else if (entrys.compare(Closer) == CPEQUAL)
		{
			openArrays--;
			if (openArrays <= 0)
				return ++position;
		}
		position++;
	}
	return 0;
}
	