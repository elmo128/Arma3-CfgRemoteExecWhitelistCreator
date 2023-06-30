// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#include <string>
#include <regex>
#include <filesystem>
#include <map>
#include <iostream>
#include <future>
#include <fstream>
#include <deque>
#include <ostream>

#include "Processor.h"
#include "main.h"

void Processor::Run(void)
{
	while (UI());
	UIProcessing();
	UI_End();
}

int Processor::UI(void)
{
	std::cout << "Welcome to A3 remoteexec whitelist creator." << std::endl;
	std::cout << "Enter the path to your mission directory, or nothing to use the current one." << std::endl;
	for (std::string input; std::getline(std::cin, input);)
	{
		if (!input.empty())
		{
			Settings_.missionRoot = input;
			if (std::filesystem::exists(Settings_.missionRoot))
			{
				std::cout << "OK";
				break;
			}
			else
			{
				Settings_.missionRoot.clear();
				std::cout << "Invalid path! try again!" << std::endl;
			}
		}
		else
		{
			Settings_.missionRoot = std::filesystem::current_path();
			std::cout << "current path: " << Settings_.missionRoot.string();
			break;
		}
	}

	std::cout << std::endl << std::endl;
	std::cout << "If you want logfiles to be analyzed to improve the result, enter their parent directory (non recursive search for .rpt files) or press enter to continue." << std::endl;
	for (std::string input; std::getline(std::cin, input);)
	{
		if (!input.empty())
		{
			Settings_.LogfileDir = input;
			if (std::filesystem::exists(Settings_.LogfileDir))
			{
				std::cout << "OK";
				break;
			}
			else
			{
				Settings_.LogfileDir.clear();
				std::cout << "Invalid path! try again!" << std::endl;
			}
		}
		else
		{
			std::cout << "No path, logfiles won't be used!";
			break;
		}
	}

	std::cout << std::endl << std::endl;
	std::cout << "Do you want to enter options (no delimiter required, case insensitive) do so now." << std::endl
		<< "s = Disable multithreading." << std::endl
		<< "j = Consider jip value when processing logfiles." << std::endl
		<< "f = Write config to selected file." << std::endl
		<< "i = Ignore existing config." << std::endl
		<< "l = Print a list of all files that are processed to the terminal." << std::endl
		<< "h = Print info" << std::endl;

	for (std::string input; std::getline(std::cin, input);)
	{
		if (!input.empty())
		{
			std::cout << "Selected options:" << std::endl;
			if (input.find_first_of("sS") != std::string::npos)
			{
				Settings_.multithreading = false;
				std::cout << "s: Multithreading disabled!" << std::endl;
			}
			if (input.find_first_of("jJ") != std::string::npos)
			{
				Settings_.considerJip = true;
				std::cout << "j: Logfiles: Jip value enabled!" << std::endl;
			}
			if (input.find_first_of("fF") != std::string::npos)
			{
				Settings_.writeCfg2File = true;
				std::cout << "f: appending config to specified file!" << std::endl;
			}
			if (input.find_first_of("iI") != std::string::npos)
			{
				Settings_.ignoreConfig = true;
				std::cout << "i: The existing config will be ignored!" << std::endl;
			}
			if (input.find_first_of("lL") != std::string::npos)
			{
				Settings_.debugLevel_ = 2;
				std::cout << "l: Print all processed files to the terminal!" << std::endl;
			}
			if (input.find_first_of("hH") != std::string::npos)
			{
				std::cout << "h: Print Info!" << std::endl;
				std::cout << "Download source: "
					<< "https://github.com/elmo128/Arma3-CfgRemoteExecWhitelistCreator" << std::endl
					<< "MIT License:\nCopyright(c) 2023 elmo128\nPermission is hereby granted, free of charge, to any person obtaining a copy\nof this software and associated documentation files(the \"Software\"), to deal\nin the Software without restriction, including without limitation the rights\nto use, copy, modify, merge, publish, distribute, sublicense, and /or sell\ncopies of the Software, and to permit persons to whom the Software is\nfurnished to do so, subject to the following conditions :\n\nThe above copyright notice and this permission notice shall be included in all\ncopies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE."
					<< std::endl << std::endl;
				return 1;
			}
			std::cout << "OK";
			break;
		}
		else
		{
			std::cout << "No Options used!";
			break;
		}
	}

	if (Settings_.writeCfg2File)
	{
		std::cout << std::endl << std::endl;
		std::cout << "Specify the file the config should be written to." << std::endl;
		for (std::string input; std::getline(std::cin, input);)
		{
			if (!input.empty())
			{
				Settings_.OutputFile_ = input;
				if (std::filesystem::exists(Settings_.OutputFile_))
				{
					std::cout << "overwriting config if available, or appending a new one." << std::endl;
					break;
				}
				else
				{
					Settings_.OutputFile_.clear();
					std::cout << "Invalid path! File will be created recursiveley!" << std::endl;
					break;
				}
			}
		}
	}

	std::cout << std::endl << std::endl;
	std::cout << "Press Enter to start processing!";
	for (std::string input; std::getline(std::cin, input);)
	{
		std::cout << "Processing now:" << std::endl;
		break;
	}
	return 0;
}
void Processor::UIProcessing(void)
{
	std::cout << "Scanning files..." << std::endl;
	retreiveData();
	std::cout << std::endl << "Merging data..." << std::endl;
	mergeData();
	std::cout << "Writing data..." << std::endl;
	writeData();
	std::cout << "Operation complete!" << std::endl;
}
void Processor::UI_End(void)
{
	std::cout << "Press enter to exit!";
	for (std::string input; std::getline(std::cin, input);)
	{
		break;
	}
}

void Processor::retreiveData(void)
{
	if (Settings_.multithreading)
	{
		auto ffres = std::async(std::launch::async, &FileAnalyzer::getDataDir, this, Settings_.missionRoot, Settings_.debugLevel_);
		if (!Settings_.ignoreConfig)
		{
			auto fcfgres = std::async(std::launch::async, &ConfigAnalyzer::retreiveConfig, this, Settings_.missionRoot, Settings_.multithreading, Settings_.debugLevel_);
			if (fcfgres.valid())
			{
				fcfgres.wait();
				configResults_ = fcfgres.get();
			}
		}
		if (std::filesystem::exists(Settings_.LogfileDir))
		{
			auto flfres = std::async(std::launch::async, &LogfileAnalyzer::ProcessLogfilesP, this, Settings_.LogfileDir, Settings_.multithreading, Settings_.debugLevel_);
			if (flfres.valid())
			{
				flfres.wait();
				logfileResults_ = flfres.get();
			}
		}
		if (ffres.valid())
		{
			ffres.wait();
			try
			{
				fileResults_ = ffres.get();
			}
			catch (std::exception ex)
			{
				std::cout << ex.what() << std::endl;
				throw ex;
			}
		}
	}
	else
	{
		fileResults_ = getDataDir(Settings_.missionRoot, Settings_.debugLevel_);
		if (!Settings_.ignoreConfig)
		{
			configResults_ = retreiveConfig(Settings_.missionRoot, Settings_.debugLevel_);
		}
		if (std::filesystem::exists(Settings_.LogfileDir))
		{
			logfileResults_ = ProcessLogfiles(Settings_.LogfileDir, Settings_.multithreading, Settings_.debugLevel_);
		}
	}
}
void Processor::mergeData(void)
{
	// handle sections and modes
	if (configResults_.contains(Helper::fnName2Key("Functions")))
	{
		auto cfgEntry = configResults_.at(Helper::fnName2Key("Functions"));
		jipWeakFN_ = cfgEntry.JipWeak;
		modeFN_ = cfgEntry.remoteexecMode;
	}
	else if (configResults_.contains(Helper::fnName2Key("Commands")))
	{
		auto cfgEntry = configResults_.at(Helper::fnName2Key("Commands"));
		jipWeakCMD_ = cfgEntry.JipWeak;
		modeCMD_ = cfgEntry.remoteexecMode;
	}

	if (jipWeakFN_ < 0 || Settings_.jipWeakFN_ >= 0)jipWeakFN_ = Settings_.jipWeakFN_;
	if (modeFN_ < 0 || Settings_.modeFN_ >= 0)modeFN_ = Settings_.modeFN_;
	if (jipWeakCMD_ < 0 || Settings_.jipWeakCMD_ >= 0)jipWeakCMD_ = Settings_.jipWeakCMD_;
	if (modeCMD_ < 0 || Settings_.modeCMD_ >= 0)modeCMD_ = Settings_.modeCMD_;

	//handle overwrites
	for (auto& [key, Order] : fileResults_)
	{
		// fix filesystem analysis by using results from logfile
		if (std::filesystem::exists(Settings_.LogfileDir))
		{
			if (logfileResults_.contains(Helper::fnName2Key(key)))
			{
				auto logEntry = logfileResults_.at(Helper::fnName2Key(key));
				Order.jip = logEntry.getJipAllowed();
				Order.target = logEntry.getTargetsAllowed();
			}
		}
		// improve fs analysis by appplying config results
		if (!Settings_.ignoreConfig)
		{
			if (configResults_.contains(Helper::fnName2Key(key)))
			{
				auto cfgEntry = configResults_.at(Helper::fnName2Key(key));
				Order.jip = cfgEntry.jip;
				Order.target = cfgEntry.allowedtargets;
			}
		}
	}
	// add missing config entries (assume config is final!)
	for (auto& [key, Order] : configResults_)
	{
		if (!Settings_.ignoreConfig)
		{
			if (!fileResults_.contains(Helper::fnName2Key(key)))
			{
				FileAnalyzer::OrderParams<int, int> FOrder;
				FOrder.isfuction = Order.isfuction;
				FOrder.jip = Order.jip;
				FOrder.Order = Order.Order;
				FOrder.target = Order.allowedtargets;
				fileResults_.insert(std::make_pair(Helper::fnName2Key(key), FOrder));
			}
		}
	}
}
void Processor::writeData(void)
{
	if (Settings_.writeCfg2File)
	{
		if (std::filesystem::exists(Settings_.OutputFile_)) // replace if found or append
		{
			std::ifstream fromFile(Settings_.OutputFile_, std::ios::in);
			auto tmppath = createTempfilePath(Settings_.OutputFile_);
			std::ofstream toFile(tmppath, std::ios::out | std::ios::app);
			bool found = false;
			if (fromFile.is_open() && fromFile.good() && toFile.is_open() && toFile.good())
			{
				int openBrackets = 0;
				for (std::string line; std::getline(fromFile, line); )
				{
					auto match = findCfgRemoteExec(&line);
					if (!match.empty())
					{
						toFile << match.prefix().str();
						line = match.suffix().str();
						writeConfig(toFile);
						found = true;
					};
					openBrackets = handleBrackets(&line, openBrackets);
					if (openBrackets <= 0)
						toFile << line << std::endl;
				}
			}
			fromFile.close();
			toFile.close();
			if (found)
			{
				auto permis = std::filesystem::status(Settings_.OutputFile_).permissions();
				std::filesystem::remove(Settings_.OutputFile_);
				std::filesystem::permissions(tmppath, permis);
				std::filesystem::rename(tmppath, Settings_.OutputFile_);
			}
			else
			{
				std::filesystem::remove(tmppath);

				std::ofstream toFile(Settings_.OutputFile_, std::ios::out | std::ios::app);
				if (toFile.is_open() && toFile.good())
					writeConfig(toFile);
				toFile.close();
			}
		}
		else // create recursiveley and append
		{
			std::filesystem::create_directories(Settings_.OutputFile_.parent_path());
			std::ofstream toFile(Settings_.OutputFile_, std::ios::out | std::ios::app);
			if (toFile.is_open() && toFile.good())
				writeConfig(toFile);
			toFile.close();
		}
	}
	else
	{
		writeConfig(std::cout);
	}
}
std::smatch Processor::findCfgRemoteExec(std::string* line)
{
#ifdef _MSC_VER
	std::regex regex2find("((class\\s+CfgRemoteExec))", std::regex::ECMAScript | std::regex::icase);
#else
	std::regex regex2find("((class\\s+CfgRemoteExec))", std::regex::ECMAScript | std::regex::icase);
#endif 
	std::smatch regexMatch;
	std::regex_search(*line, regexMatch, regex2find);
	return regexMatch;
}
int Processor::handleBrackets(std::string* str, int openBrackets)
{
	size_t offsetOpener = -1;
	size_t offsetCloser = -1;
	std::deque<std::pair<size_t, size_t>> indices;
	do
	{
		offsetOpener = str->find("{", offsetOpener + 1);
		if (offsetOpener != std::string::npos)
		{
			openBrackets++;
		}
		offsetCloser = str->find("}", offsetCloser + 1);
		if (offsetCloser != std::string::npos)
		{
			openBrackets--;
		}
		if (offsetOpener < offsetCloser)
			indices.push_front(std::make_pair(offsetOpener, offsetCloser));

	} while (openBrackets > 0 && (offsetOpener != std::string::npos || offsetCloser != std::string::npos));

	for (auto& [front, back] : indices)
	{
		str->erase(front, back - front);
	}
	return openBrackets;
}
void Processor::writeConfig(std::ostream& outstream)
{
	outstream << "class CfgRemoteExec" << std::endl
		<< "{" << std::endl
		<< "\t" << "class Functions" << std::endl
		<< "\t" << "{" << std::endl;
	if (modeFN_ >= 0)
		outstream << "\t\t" << "mode = " << std::to_string(modeFN_) << ";" << std::endl;
	if (jipWeakFN_ >= 0)
		outstream << "\t\t" << "jip = " << std::to_string(jipWeakFN_) << ";" << std::endl;

	for (auto& [key, value] : fileResults_)
	{
		if ((key.compare(Helper::fnName2Key("Functions")) == CPEQUAL)
			|| (key.compare(Helper::fnName2Key("Commands")) == CPEQUAL)
			|| (key.compare(Helper::fnName2Key("\"\"")) == CPEQUAL)) continue;
		if (value.isfuction)
		{
			outstream << "\t\t" << "class " << Helper::cleanupFunctionname(value.Order) << " {";
			if (value.target >= 0)
				outstream << " allowedTargets = " << std::to_string(value.target) << ";";
			if ((value.jip != jipWeakFN_) && (value.jip >= 0))
				outstream << " jip = " << std::to_string(value.jip) << ";";
			outstream << " };" << std::endl;
		}
	}
	outstream << "\t" << "};" << std::endl;

	outstream << "\t" << "class Commands" << std::endl
		<< "\t" << "{" << std::endl;
	if (modeCMD_ >= 0)
		outstream << "\t\t" << "mode = " << std::to_string(modeCMD_) << ";" << std::endl;
	if (jipWeakCMD_ >= 0)
		outstream << "\t\t" << "jip = " << std::to_string(jipWeakCMD_) << ";" << std::endl;

	for (auto& [key, value] : fileResults_)
	{
		if ((key.compare(Helper::fnName2Key("Functions")) == CPEQUAL)
			|| (key.compare(Helper::fnName2Key("Commands")) == CPEQUAL)
			|| (key.compare(Helper::fnName2Key("\"\"")) == CPEQUAL)) continue;
		if (!value.isfuction)
		{
			outstream << "\t\t" << "class " << Helper::cleanupFunctionname(value.Order) << " {";
			if (value.target >= 0)
				outstream << " allowedTargets = " << std::to_string(value.target) << ";";
			if ((value.jip != jipWeakCMD_) && (value.jip >= 0))
				outstream << " jip = " << std::to_string(value.jip) << ";";
			outstream << " };" << std::endl;
		}
	}
	outstream << "\t" << "};" << std::endl;
	outstream << "};" << std::endl;
}
std::filesystem::path Processor::createTempfilePath(std::filesystem::path sourceFile)
{
	int i = 0;
	while (std::filesystem::exists(sourceFile))
	{
		sourceFile = sourceFile.string() + ".tmp" + std::to_string(i);
		i++;
	}
	return sourceFile;
}
