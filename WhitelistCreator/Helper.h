// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#pragma once

#include <string>
#include <vector>

class Helper
{
public:
	bool remSinglelineComments(std::string* data, bool inCommentMultipleRead);
	bool remMultilineComments(std::string* data, bool inCommentMultipleRead = false);
	bool caseInsensitiveStrEqual(const std::string* first, const std::vector<std::string>* second);
	bool caseInsensitiveStrEqual(const std::string* first, const std::string* second);
	std::string cleanupPath(std::string str);
	void removeWSLB(std::string* str);
	void removeMWSLB(std::string* str);
	void cleanupString(std::string* str);
	template<typename T>
	T lower(T str) requires((std::is_same_v<T, std::wstring>) || (std::is_same_v<T, std::string>))
	{
		T outstr;
		for (auto& entry : str)
		{
			if (std::is_same_v<T, std::wstring> == true)
			{
				outstr += towlower(entry);
			}
			else
			{
				outstr += std::tolower(entry);
			}
		}
		return outstr;
	}
	std::string cleanupFunctionname(std::string fname);
	std::string fnName2Key(std::string name);
};
