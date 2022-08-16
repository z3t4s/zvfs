#pragma once
#include <algorithm>
#include <string>

namespace zvfs
{
	namespace path
	{
		extern std::pair<std::string_view, std::string_view> split_path(std::string_view path);

		extern std::pair<std::string_view, std::string_view> split_path(std::string& path);
	}
}