// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#pragma once

#include <map>
#include <regex>
#include <filesystem>

#include "Helper.h"

class FileAnalyzer : Helper
{
public:
	FileAnalyzer() :debugLevel_(0) {};
	template<class T, class J> requires (((std::is_same_v<T, int>) || (std::is_same_v<T, std::string>)) && ((std::is_same_v<J, int>) || (std::is_same_v<J, std::string>)))
	class OrderParams
	{
	public:
		OrderParams()
		{
			isfuction = false;
			Order = "";
			if constexpr (std::is_same_v<T, int>)
			{
				target = -1;
			}
			if constexpr (std::is_same_v<J, int>)
			{
				jip = -1;
			}
			if constexpr (std::is_same_v<T, std::string>)
			{
				target = "";
			}
			if constexpr (std::is_same_v<J, std::string>)
			{
				jip = "";
			}
		}
		bool isfuction;
		std::string Order;
		T target;
		J jip;

		template<typename T2, typename J2>
		void getfromOtherVariant(OrderParams<T2, J2> input)
		{
			isfuction = input.isfuction;
			Order = input.Order;
			if constexpr (std::is_same_v<T, T2>)
			{
				target = input.target;
			}
			if constexpr (std::is_same_v<J, J2>)
			{
				jip = input.jip;
			}
		}
	};
	std::map<std::string, FileAnalyzer::OrderParams<int, int>> getDataDir(std::filesystem::path path, int dbgPrint = false);
	std::map<std::string, FileAnalyzer::OrderParams<int, int>> getDataFile(std::filesystem::path path, int dbgPrint = false);
	std::map<std::string, FileAnalyzer::OrderParams<int, int>> getDataLine(const std::string* line, std::filesystem::path path = "");
	void setDebugLevel(int level) { debugLevel_ = level; };
private:
	int debugLevel_;

	std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> ParseDir(std::filesystem::path path);
	std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> parseFile(std::filesystem::path path);
	std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> ParseLine(const std::string* line);
	std::vector<std::string> orderWalker(std::string* line);
	std::smatch findPrimaryArray(std::string* line);
	FileAnalyzer::OrderParams<std::string, std::string> getArguments(std::vector<std::string> elements);
	bool isfunction(std::string* order);
	std::map<std::string, FileAnalyzer::OrderParams<int, int>> Interpreter(std::multimap<std::string, FileAnalyzer::OrderParams<std::string, std::string>> map);
	FileAnalyzer::OrderParams<int, std::string> getmaxRequiredRights(std::vector<FileAnalyzer::OrderParams<std::string, std::string>>* data);
	FileAnalyzer::OrderParams<std::string, int> getJipFlag(std::vector<FileAnalyzer::OrderParams<std::string, std::string>>* data);
	int parseNumber(std::string target);
	std::vector<std::string> parseArray(const std::string* line);
	int getfinalCloser(const std::string* line, const std::string Opener, const std::string Closer);
};
