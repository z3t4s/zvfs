#include "path.hpp"
namespace zvfs
{
	namespace path
	{
		std::pair<std::string_view, std::string_view> split_path(std::string_view path)
		{
			// Check for a trailing slash that would indicate the last "directory"
			// If split_index is zero, we don't have a "directory" in the path
			//
			size_t split_index = path.substr(0, path.size() - 1).rfind('/') + 1;

			return { path.substr(0, split_index), path.substr(split_index) };
		}

		std::pair<std::string_view, std::string_view> split_path(std::string& path)
		{
			std::string_view path_view = path;
			size_t split_index = path.substr(0, path_view.size() - 1).rfind('/') + 1;

			return { path_view.substr(0, split_index), path_view.substr(split_index) };
		}
	} // namespace path
} // namespace zvfs