// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#pragma once

#include <map>
#include <vector>
#include <filesystem>

#include "Helper.h"
#include "main.h"

class LogfileAnalyzer : Helper
{
public:
	LogfileAnalyzer() :debugLevel_(0) {};
	class exeResults
	{
	public:
		exeResults() :functionName_(""), jipAllowed_(-1), target_(-1) {};
		std::string getFunction(void) { return functionName_; };
		int getJipAllowed(void) { return jipAllowed_; };
		int getTargetsAllowed(void) { return target_; };
		void setFunction(std::string data) { functionName_ = data; };
		void setJipAllowed(int data) { jipAllowed_ = data; };
		void setTargetsAllowed(int data) { target_ = data; };
		void clear(void) { functionName_.clear(); jipAllowed_ = -1; target_ = -1; };
	private:
		std::string functionName_;
		int jipAllowed_;
		int target_;
	};
	std::map<std::string, exeResults> ProcessLogfilesP(std::filesystem::path paths, bool mulicore = true, int dbgPrint = 0);
	template<typename T>
	std::map<std::string, exeResults> ProcessLogfiles(T paths, bool mulicore = true, int dbgPrint = 0, std::vector<std::string> extensions = LOGFILE_EXTENSIONS)requires ((std::is_same_v<T, std::filesystem::path>) || (std::is_same_v<T, std::vector<std::filesystem::path>>))
	{
		debugLevel_ = dbgPrint;
		std::vector<std::filesystem::path> pathlist;
		std::filesystem::path ptype;
		if (std::is_same_v<decltype(ptype), decltype(paths)> == true)
		{
			for (auto const& entry : std::filesystem::directory_iterator(paths))
			{
				auto pth = entry.path().extension().string();
				if (entry.is_regular_file() && entry.path().has_extension() && caseInsensitiveStrEqual(&pth, &extensions))
				{
					pathlist.push_back(entry);
				}
			}
		}
		else
		{
			for (auto const& entry : paths)
			{
				auto pth = entry.extension().string();
				if (std::filesystem::is_regular_file(entry) && entry.has_extension() && caseInsensitiveStrEqual(&pth, &extensions))
				{
					pathlist.push_back(entry);
				}
			}
		}
		if (mulicore)
		{
			return processLogEntries(FileAnalysis(pathlist, true), true);
		}
		else
		{
			return processLogEntries(FileAnalysis(pathlist));
		}
	}
	void setDebugLevel(int level) { debugLevel_ = level; };
private:
	int debugLevel_;
	std::multimap<std::string, exeResults> FileAnalysis(std::vector<std::filesystem::path> paths, bool multithreading = false);
	std::multimap<std::string, exeResults> analyzeLogfile(std::filesystem::path path);
	bool lineisEntry(std::string* line);
	std::pair<std::string, exeResults> parseLine(std::string* line);
	std::map<std::string, exeResults> processLogEntries(std::multimap<std::string, exeResults> dataMap, bool multithreading = false);
	exeResults processEntries(std::pair<std::map<std::string, LogfileAnalyzer::exeResults>::iterator, std::map<std::string, LogfileAnalyzer::exeResults>::iterator> range);
	int parseValue(std::string* element);
};
