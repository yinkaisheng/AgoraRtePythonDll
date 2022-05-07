#include "util.h"
#include <windows.h>
#include <memory>
#include <io.h>

#define STRING_BUFFER_SIZE 512

bool IsPathExist(const char* path)
{
    return _access(path, 0) == 0;
}

bool IsPathExist(const wchar_t* path)
{
    return _waccess(path, 0) == 0;
}

bool MakeDirsExist(std::wstring_view path)
{
    size_t start = 0;
    std::wstring sub;
    while (start < path.size())
    {
        size_t it = path.find_first_of(L"/\\", start);
        if (it != path.npos)
        {
            sub = path.substr(0, it + 1);
            start = it + 1;
        }
        else
        {
            sub = path;
            start = path.size();
        }

        struct _stat st;
        if (_wstat(sub.c_str(), &st) == 0 && (st.st_mode & S_IFDIR))
        {
            wprintf(L"sub dir exists: %s\n", sub.c_str());
        }
        else
        {
            int ret = _wmkdir(sub.c_str());
            if (ret != 0)
            {
                return false;
            }
            else
            {
                wprintf(L"create ok for path: %s\n", sub.c_str());
            }
        }
    }

    return true;
}

std::wstring Ansi2Wide(std::string_view ansi)
{
    //if the 3rd parameter is -1, the return len includes \0
    int len = MultiByteToWideChar(CP_ACP, 0, ansi.data(), ansi.size(), nullptr, 0);
    if (len == 0)
    {
        DWORD lastError = GetLastError();
        printf(__FUNCTION__ " MultiByteToWideChar LastError=%lu\n", lastError);
        return L"";
    }
    ++len;  //the buffer len doesn't contain null character \0, need to add 1
    std::unique_ptr<wchar_t[]> result{ new wchar_t[len] {} };
    len = MultiByteToWideChar(CP_ACP, 0, ansi.data(), ansi.size(), result.get(), len);
    return result.get();
}

int Ansi2Wide(std::string_view ansi, wchar_t* wszOut, int outSize)
{
    int len = MultiByteToWideChar(CP_ACP, 0, ansi.data(), ansi.size(), wszOut, outSize-1);
    wszOut[len] = 0;
    return len;
}

std::string Wide2Ansi(std::wstring_view wide)
{
    int len = WideCharToMultiByte(CP_ACP, 0, wide.data(), wide.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0)
    {
        DWORD lastError = GetLastError();
        printf(__FUNCTION__ " WideCharToMultiByte LastError=%lu\n", lastError);
        return "";
    }
    ++len;  //the buffer len doesn't contain null character \0, need to add 1
    std::unique_ptr<char[]> result{ new char[len] {} };
    len = WideCharToMultiByte(CP_ACP, 0, wide.data(), wide.size(), result.get(), len, nullptr, nullptr);
    return result.get();
}

int Wide2Ansi(std::wstring_view wide, char* szOut, int outSize)
{
    int len = WideCharToMultiByte(CP_ACP, 0, wide.data(), wide.size(), szOut, outSize-1, nullptr, nullptr);
    szOut[len] = 0;
    return len;
}

std::wstring Utf82Wide(std::string_view utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), nullptr, 0);
    if (len == 0)
    {
        DWORD lastError = GetLastError();
        printf(__FUNCTION__ " MultiByteToWideChar LastError=%lu\n", lastError);
    }
    ++len;  //the buffer len doesn't contain null character \0, need to add 1
    std::unique_ptr<wchar_t[]> result{ new wchar_t[len] {} };
    len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), result.get(), len);
    return result.get();
}

int Utf82Wide(std::string_view utf8, wchar_t* wszOut, int outSize)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), wszOut, outSize-1);
    wszOut[len] = 0;
    return len;
}

std::string Wide2Utf8(std::wstring_view wide)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide.size(), nullptr, 0, nullptr, nullptr);
    if (len == 0)
    {
        DWORD lastError = GetLastError();
        printf(__FUNCTION__ " WideCharToMultiByte LastError=%lu\n", lastError);
        return "";
    }
    ++len;  //the buffer len doesn't contain null character \0, need to add 1
    std::unique_ptr<char[]> result{ new char[len] {} };
    len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide.size(), result.get(), len, nullptr, nullptr);
    return result.get();
}

int Wide2Utf8(std::wstring_view wide, char* szOut, int outSize)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wide.size(), szOut, outSize-1, nullptr, nullptr);
    szOut[len] = 0;
    return len;
}

std::string Ansi2Utf8(std::string_view ansi)
{
    return Wide2Utf8(Ansi2Wide(ansi));
}

int Ansi2Utf8(std::string_view ansi, char* szOut, int outSize)
{
    return Wide2Utf8(Ansi2Wide(ansi), szOut, outSize);
}

std::string Utf82Ansi(std::string_view ansi)
{
    return Wide2Ansi(Utf82Wide(ansi));
}

int Utf82Ansi(std::string_view ansi, char* szOut, int outSize)
{
    return Wide2Ansi(Utf82Wide(ansi), szOut, outSize);
}

long long EpochMicroseconds()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER epoch; // UNIX epoch (1970-01-01 00:00:00) expressed in Windows NT FILETIME
    epoch.LowPart = 0xD53E8000;
    epoch.HighPart = 0x019DB1DE;

    ULARGE_INTEGER ts;
    ts.LowPart = ft.dwLowDateTime;
    ts.HighPart = ft.dwHighDateTime;
    ts.QuadPart -= epoch.QuadPart;
    return ts.QuadPart / 10;
}


std::string String::format(std::string_view format, ...)
{
	std::string result(STRING_BUFFER_SIZE, '\0');
	size_t nSize = STRING_BUFFER_SIZE;
	int len = 0;
	int ret = -1;

	while (true)
	{
		va_list args;
		va_start(args, format);
#if defined _MSC_VER && _MSC_VER < 1900
		ret = _vsnprintf_s(result.data(), result.size(), -1, format.data(), args); // return -1 if size is not enough
#else
		// from VS2015, vsnprintf return buffer size needed if buffer is not enough
		// do not use vsnprintf if MSVC < VS2015, if it returns -1, the buffer may be not null terminated.
		ret = vsnprintf(result.data(), result.size(), format.data(), args);
#endif
		va_end(args);
		if (ret >= 0 && (uint32_t)ret < nSize)
		{
			len = ret;
			break;
		}
		else
		{
			len = nSize - 1;
#if defined _MSC_VER && _MSC_VER < 1900
			nSize *= 2;
#else
			nSize = ret + 1;
#endif
			result.resize(nSize);
		}
	}

	return result;
}

bool String::startsWith(std::string_view str, std::string_view start)
{
	return str.find(start) == 0;
}

bool String::endsWith(std::string_view str, std::string_view end)
{
	if (str.size() < end.size())
	{
		return false;
	}

	return str.substr(str.size() - end.size()) == end;
}

bool String::contains(std::string_view src, std::string_view part)
{
	return src.find(part) != std::string_view::npos;
}

std::string String::trim(std::string_view src)
{
	size_t start = src.find_first_not_of(" \t\r\n");
	if (start == std::string_view::npos)
		return "";

	size_t end = src.find_last_not_of(" \t\r\n");
	if (end == std::string_view::npos)
		return "";

	auto sv = src.substr(start, end - start + 1);
	return std::string(sv.data(), sv.size());
}

std::wstring String::trim(std::wstring_view src)
{
	size_t start = src.find_first_not_of(L" \t\r\n");
	if (start == std::wstring_view::npos)
		return L"";

	size_t end = src.find_last_not_of(L" \t\r\n");
	if (end == std::wstring_view::npos)
		return L"";

	auto sv = src.substr(start, end - start + 1);
	return std::wstring(sv.data(), sv.size());
}


std::string String::trimLeft(std::string_view src)
{
	size_t start = src.find_first_not_of(" \t\r\n");
	if (start == std::string_view::npos)
		return "";

	auto sv = src.substr(start);
	return std::string(sv.data(), sv.size());
}

std::string String::trimRight(std::string_view src)
{
	size_t end = src.find_last_not_of(" \t\r\n");
	if (end == std::string_view::npos)
		return "";

	auto sv = src.substr(0, end + 1);
	return std::string(sv.data(), sv.size());
}

std::wstring String::trimRight(std::wstring_view src)
{
	size_t end = src.find_last_not_of(L" \t\r\n");
	if (end == std::wstring_view::npos)
		return L"";

	auto sv = src.substr(0, end + 1);
	return std::wstring(sv.data(), sv.size());
}

int String::replace(std::string& str, std::string_view before, std::string_view after)
{
	int replaceTimes = 0;

	for (size_t start = 0, index = 0;
		index = str.find(before.data(), start, before.size()), index != std::string::npos;
		start = index + after.size())
	{
		str.replace(index, before.size(), after.data(), after.size());
		++replaceTimes;
	}

	return replaceTimes;
}

int String::replace(std::wstring& str, std::wstring_view before, std::wstring_view after)
{
	int replaceTimes = 0;

	for (size_t start = 0, index = 0;
		index = str.find(before.data(), start, before.size()), index != std::wstring::npos;
		start = index + after.size())
	{
		str.replace(index, before.size(), after.data(), after.size());
		++replaceTimes;
	}

	return replaceTimes;
}

size_t String::split(std::string_view src, std::vector<std::string>& parts, std::string_view splitter/* = ""*/, bool splitterAsEntirety/* = true*/, size_t maxCount/* = -1*/)
{
	parts.clear();
	size_t index = 0;
	size_t start = 0, left = 0;

	if (splitter.empty())
	{
		splitter = " \t\r\n";
		splitterAsEntirety = false;
	}

	while (true)
	{
		if (splitterAsEntirety)
		{
			index = src.find(splitter, left);
		}
		else
		{
			index = src.find_first_of(splitter, left);
		}

		if (index == std::string::npos)
		{
			if (src.size() > left)
			{
				auto sv = src.substr(left, src.size() - left);
				parts.emplace_back(sv.data(), sv.size());
				//parts.push_back(src.substr(left, src.size() - index));
			}

			break;
		}
		else
		{
			if (index > left)
			{
				auto sv = src.substr(left, index - left);
				parts.emplace_back(sv.data(), sv.size());
				//parts.push_back(src.substr(left, index - index));
				left = index + (splitterAsEntirety ? splitter.size() : 1);
			}
			else// index == left
			{
				left += splitterAsEntirety ? splitter.size() : 1;
			}
		}
	}

	return parts.size();
}

size_t String::split(std::wstring_view src, std::vector<std::wstring>& parts, std::wstring_view splitter/* = ""*/, bool splitterAsEntirety/* = true*/, size_t maxCount/* = -1*/)
{
	parts.clear();
	size_t index = 0;
	size_t start = 0, left = 0;

	if (splitter.empty())
	{
		splitter = L" \t\r\n";
		splitterAsEntirety = false;
	}

	while (true)
	{
		if (splitterAsEntirety)
		{
			index = src.find(splitter, left);
		}
		else
		{
			index = src.find_first_of(splitter, left);
		}

		if (index == std::wstring::npos)
		{
			if (src.size() > left)
			{
				auto sv = src.substr(left, src.size() - left);
				parts.emplace_back(sv.data(), sv.size());
				//parts.push_back(src.substr(left, src.size() - index));
			}

			break;
		}
		else
		{
			if (index > left)
			{
				auto sv = src.substr(left, index - left);
				parts.emplace_back(sv.data(), sv.size());
				//parts.push_back(src.substr(left, index - index));
				left = index + (splitterAsEntirety ? splitter.size() : 1);
			}
			else// index == left
			{
				left += splitterAsEntirety ? splitter.size() : 1;
			}
		}
	}

	return parts.size();
}

std::string String::join(std::string_view splitter, const std::vector<std::string>& parts)
{
	return join(splitter, parts.cbegin(), parts.cend());
}

std::string String::join(std::string_view splitter, std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end)
{
	std::string result;

	for (; begin != end; ++begin)
	{
		result += *begin;

		if (begin + 1 != end)
		{
			result += splitter;
		}
	}

	return result;
}

std::string String::between(std::string_view src, std::string_view left, std::string_view right/* = ""*/,
	size_t start/* = 0*/, size_t* index/* = nullptr*/, size_t* nextStart/* = nullptr*/)
{
	if (left.empty())
	{
		if (right.empty())
		{
			if (index) *index = -1;
			//if (nextStart) *nextStart = -1;
			return "";
		}
		else
		{
			size_t rindex = src.find(right, start);

			if (rindex == std::string_view::npos)
			{
				if (index) *index = -1;
				//if (nextStart) *nextStart = -1;
				return "";
			}
			else
			{
				if (index) *index = start;
				if (nextStart) *nextStart = rindex + right.size();
				auto sv = src.substr(start, rindex - start);
				return std::string(sv.data(), sv.size());
			}
		}
	}
	else
	{
		size_t lindex = src.find(left, start);
		if (lindex == std::string_view::npos)
		{
			if (index) *index = -1;
			//if (nextStart) *nextStart = -1;
			return "";
		}
		else
		{
			if (right.empty())
			{
				if (index) *index = lindex + left.size();
				if (nextStart) *nextStart = src.size();
				auto sv = src.substr(lindex + left.size());
				return std::string(sv.data(), sv.size());
			}
			else
			{
				size_t rindex = src.find(right, lindex + left.size());

				if (rindex == std::string_view::npos)
				{
					if (index) *index = -1;
					//if (nextStart) *nextStart = -1;
					return "";
				}
				else
				{
					if (index) *index = lindex + left.size();
					if (nextStart) *nextStart = rindex + right.size();
					auto sv = src.substr(lindex + left.size(), rindex - lindex - left.size());
					return std::string(sv.data(), sv.size());
				}
			}
		}
	}

	//return "";
}

#ifdef _DEBUG
void String::testString()
{
	std::string as(40, 'a');
	auto format = String::format("hello");
	format = String::format("hello %d", 10);
	format = String::format("hello %s %d", as.c_str(), 1000);
	std::vector<std::string> parts;
	bool ok = String::startsWith("hello world", "he");
	ok = String::startsWith("hello world", " he");
	ok = String::startsWith("", "");
	ok = String::endsWith("hello world", "ld");
	ok = String::endsWith("hello world", " ld ");
	ok = String::endsWith("", "");

	auto res = String::trim("");
	res = String::trim(" \t\n");
	res = String::trim(" \t\nhello  ");
	res = String::trimLeft("");
	res = String::trimLeft(" \t\n");
	res = String::trimLeft(" \t\nhello  ");
	res = String::trimRight("");
	res = String::trimRight(" \t\n");
	res = String::trimRight(" \t\nhello  ");

	auto count = String::split("  \nhello  world\n\n", parts);
	count = String::split("  hello  world  ", parts, "  ");
	count = String::split("hello, world, ni", parts, ", ");
	count = String::split(", hello, world, ni", parts, ", ");
	count = String::split("hello, world, ni, ", parts, ", ");
	count = String::split(", hello, world, ni, ", parts, ", ");

	auto str = String::join(",", parts);
	str = String::join(", ", parts);
	str = String::join(", ", parts.begin(), parts.end());

	size_t start = 0;
	size_t index = 0;
	size_t next = 0;
	auto strb = String::between(" hello world ", " ", " ", 0, &index, &next);
	strb = String::between(" hello world ", " ", " ", 1, &index, &next);
	strb = String::between(" hello world ", " ", "", 0, &index, &next);
	strb = String::between(" hello world ", " ", "", 1, &index, &next);
	strb = String::between(" hello world ", "", " ", 0, &index, &next);
	strb = String::between(" hello world ", "", " ", 1, &index, &next);
	strb = String::between(R"(IChannel["subName"])", "\"", "\"", 0, &index, &next);
	const char* src = "func( 1, [2,4], [2,3,4], 3)";
	strb = String::between(src, "[", "]", 1, &start, &next);
	strb = String::between(src, "[", "]", start + strb.size() + 1, &start, &next);
}
#endif

