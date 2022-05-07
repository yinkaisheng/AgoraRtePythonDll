#pragma once
#include <string>
#include <string_view>
#include <vector>

bool IsPathExist(const char* path);

bool IsPathExist(const wchar_t* path);

bool MakeDirsExist(std::wstring_view path);

std::wstring Ansi2Wide(std::string_view ansi);

int Ansi2Wide(std::string_view ansi, wchar_t* wszOut, int outSize);

std::string Wide2Ansi(std::wstring_view wide);

int Wide2Ansi(std::wstring_view wide, char* szOut, int outSize);

std::wstring Utf82Wide(std::string_view utf8);

int Utf82Wide(std::string_view utf8, wchar_t* wszOut, int outSize);

std::string Wide2Utf8(std::wstring_view wide);

int Wide2Utf8(std::wstring_view wide, char* szOut, int outSize);

std::string Ansi2Utf8(std::string_view ansi);

int Ansi2Utf8(std::string_view ansi, char* szOut, int outSize);

std::string Utf82Ansi(std::string_view ansi);

int Utf82Ansi(std::string_view ansi, char* szOut, int outSize);

long long EpochMicroseconds();

class String
{
public:
	static std::string format(std::string_view format, ...);
	static bool startsWith(std::string_view str, std::string_view start);
	static bool endsWith(std::string_view str, std::string_view end);
	static bool contains(std::string_view src, std::string_view part);
	static std::string trim(std::string_view src);
	static std::wstring trim(std::wstring_view src);
	static std::string trimLeft(std::string_view src);
	static std::string trimRight(std::string_view src);
	static std::wstring trimRight(std::wstring_view src);
	static int replace(std::string& str, std::string_view before, std::string_view after);
	static int replace(std::wstring& str, std::wstring_view before, std::wstring_view after);
	static size_t split(std::string_view src, std::vector<std::string>& parts, std::string_view splitter = "", bool splitterAsEntirety = true, size_t maxCount = -1);
	static size_t split(std::wstring_view src, std::vector<std::wstring>& parts, std::wstring_view splitter = L"", bool splitterAsEntirety = true, size_t maxCount = -1);
	static std::string join(std::string_view splitter, const std::vector<std::string>& parts);
	static std::string join(std::string_view splitter, std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end);

	//if found, index is the return string pos in src, nextStart = index+return.size()+right.size()
	//if not found, index = -1, nextStart is not modified
	static std::string between(std::string_view src, std::string_view left, std::string_view right = "", size_t start = 0, size_t* index = nullptr, size_t* nextStart = nullptr);

#ifdef _DEBUG
	static void testString();
#endif
};

