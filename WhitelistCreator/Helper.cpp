// Copyright (c) 2023 elmo128 (elmo128@protonmail.com)

#include "Helper.h"

#include <string>
#include <vector>

bool Helper::remSinglelineComments(std::string* data, bool inCommentMultipleRead)
{
	do
	{
		size_t startpos = 0;
		size_t endpos = 0;
		if (inCommentMultipleRead == false)
		{
			if (data->find("//") != std::string::npos)
			{
				startpos = data->find("//");
				if (data->find('\n', startpos) != std::string::npos)
				{
					endpos = data->find('\n', startpos);
				}
				else
				{
					endpos = data->size();
					inCommentMultipleRead = true;
				}
			};
		}
		else
		{
			if (data->find('\n') != std::string::npos)
			{
				endpos = data->find('\n');
				inCommentMultipleRead = false;
			}
			else
			{
				endpos = data->size();
			}
		}
		if (startpos < endpos)
		{
			data->erase(startpos, endpos - startpos);
		}
	} while ((data->find("//") != std::string::npos) && (data->find('\n', (data->find("//"))) != std::string::npos));

	return inCommentMultipleRead;
}
bool Helper::remMultilineComments(std::string* data, bool inCommentMultipleRead /*= false*/)
{
	do
	{
		size_t startpos = 0;
		size_t endpos = 0;
		if (inCommentMultipleRead == false)
		{
			if (data->find("/*") != std::string::npos)
			{
				startpos = data->find("/*");
				if (data->find("*/", startpos) != std::string::npos)
				{
					endpos = data->find("*/", startpos) + 2;
				}
				else
				{
					endpos = data->size();
					inCommentMultipleRead = true;
				}
			};
		}
		else
		{
			if (data->find("*/") != std::string::npos)
			{
				endpos = data->find("*/") + 2;
				inCommentMultipleRead = false;
			}
			else
			{
				endpos = data->size();
			}
		}
		if (startpos < endpos)
		{
			data->erase(startpos, endpos - startpos);
		}
	} while ((data->find("/*") != std::string::npos) && (data->find("*/", (data->find("/*"))) != std::string::npos));

	return inCommentMultipleRead;
}
bool Helper::caseInsensitiveStrEqual(const std::string* first, const std::vector<std::string>* second)
{
	for (auto& entry : *second)
	{
#ifdef _MSC_VER
		if (_stricmp(first->c_str(), entry.c_str()) == 0)
#else
		if (stricmp(first->c_str(), entry.c_str()) == 0)
#endif // _MSC_VER
			return true;
	}
	return false;
}
bool Helper::caseInsensitiveStrEqual(const std::string* first, const std::string* second)
{
#ifdef _MSC_VER
	if (_stricmp(first->c_str(), second->c_str()) == 0)
#else
	if (stricmp(first->c_str(), second->c_str()) == 0)
#endif // _MSC_VER
		return true;
	else
		return false;
}

std::string Helper::cleanupPath(std::string str)
{
	for (size_t i = 0; i < 1000; i++)
	{
		if (str.starts_with(' '))
		{
			str.erase(0, 1);
		}
		else if (str.starts_with('\t'))
		{
			str.erase(0, 1);
		}
		else if (str.starts_with('\"'))
		{
			str.erase(0, 1);
		}
		else if (str.starts_with('/'))
		{
			str.erase(0, 1);
		}
		else if (str.ends_with(' '))
		{
			str.erase(str.size() - 1, 1);
		}
		else if (str.ends_with('\t'))
		{
			str.erase(str.size() - 1, 1);
		}
		else if (str.ends_with('\"'))
		{
			str.erase(str.size() - 1, 1);
		}
		else if (str.find("\\") != std::string::npos)
		{
			str.replace(str.find("\\"), 1, "/");
		}
		else if (str.find("//") != std::string::npos)
		{
			str.replace(str.find("//"), 1, "/");
		}
		else
			break;
	}
	return str;
}
void Helper::removeWSLB(std::string* str)
{
	for (int i = 1000; i > 0; i--)
	{
		if (str->find(" ") != std::string::npos)
		{
			str->erase(str->find(" "), 1);
		}
		else if (str->find("\t") != std::string::npos)
		{
			str->replace(str->find("\t"), 1, " ");
		}
		else if (str->find("\r") != std::string::npos)
		{
			str->erase(str->find("\r"), 1);
		}
		else if (str->find("\n") != std::string::npos)
		{
			str->erase(str->find("\n"), 1);
		}
		else
		{
			break;
		}
	}
}
void Helper::removeMWSLB(std::string* str)
{
	for (int i = 1000; i > 0; i--)
	{
		if (str->find("  ") != std::string::npos)
		{
			str->erase(str->find("  "), 1);
		}
		else if (str->find("\t") != std::string::npos)
		{
			str->replace(str->find("\t"), 1, " ");
		}
		else if (str->find("\r") != std::string::npos)
		{
			str->erase(str->find("\r"), 1);
		}
		else if (str->find("\n") != std::string::npos)
		{
			str->erase(str->find("\n"), 1);
		}
		else
		{
			break;
		}
	}
}
void Helper::cleanupString(std::string* str)
{
	for (int i = 1000; i > 0; i--)
	{
		if (str->starts_with(','))
		{
			str->erase(0, 1);
		}
		else if (str->starts_with(' '))
		{
			str->erase(0, 1);
		}
		else if (str->starts_with('\t'))
		{
			str->erase(0, 1);
		}
		else if (str->ends_with('\t'))
		{
			str->erase(str->size() - 1, 1);
		}
		else if (str->ends_with('\t'))
		{
			str->erase(str->size() - 1, 1);
		}
		else if (str->ends_with('\t'))
		{
			str->erase(str->size() - 1, 1);
		}
		else
			break;
	}
};
std::string Helper::cleanupFunctionname(std::string fname)
{
	size_t offset = 0;
	for (int i = 1000; i > 0; i--)
	{
		offset = fname.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", offset);
		if (offset != std::string::npos)
		{
			fname.erase(offset, 1);
		}
		else
		{
			break;
		}
	}
	return fname;
}
std::string Helper::fnName2Key(std::string name)
{
	return lower(cleanupFunctionname(name));
}
