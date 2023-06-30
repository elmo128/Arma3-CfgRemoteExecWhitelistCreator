// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#pragma once
#include <map>
#include <string>
#include <filesystem>
#include <ostream>
#include <regex>

#include "Helper.h"
#include "ConfigAnalyzer.h"
#include "LogfileAnalyzer.h"
#include "FileAnalyzer.h"

class Processor : Helper, public ConfigAnalyzer, public LogfileAnalyzer, public FileAnalyzer
{
private:
	struct UIresult
	{
		UIresult() :multithreading(true), considerJip(false), writeCfg2File(false), ignoreConfig(false), debugLevel_(0), modeFN_(-1), jipWeakFN_(-1), modeCMD_(-1), jipWeakCMD_(-1) {};
		std::filesystem::path missionRoot;
		std::filesystem::path LogfileDir;
		std::filesystem::path OutputFile_;
		bool multithreading;
		bool considerJip;
		bool writeCfg2File;
		bool ignoreConfig;
		int debugLevel_;
		int modeFN_;
		int jipWeakFN_;
		int modeCMD_;
		int jipWeakCMD_;
		bool forceOverrideJip;
	};
public:
	Processor() : modeFN_(-1), jipWeakFN_(-1), modeCMD_(-1), jipWeakCMD_(-1) {};
	void Run();
protected:
	int UI(void);
	void UIProcessing(void);
	void UI_End(void);
private:
	void retreiveData(void);
	void mergeData(void);
	void writeData(void);
	std::smatch findCfgRemoteExec(std::string* line);
	int handleBrackets(std::string* str, int openBrackets);
	void writeConfig(std::ostream& outstream);
	std::filesystem::path createTempfilePath(std::filesystem::path sourceFile);

	UIresult Settings_;
	int modeFN_;
	int jipWeakFN_;
	int modeCMD_;
	int jipWeakCMD_;
	std::map<std::string, LogfileAnalyzer::exeResults> logfileResults_;
	std::map<std::string, ConfigAnalyzer::OrderParams> configResults_;
	std::map<std::string, FileAnalyzer::OrderParams<int, int>> fileResults_;
};
