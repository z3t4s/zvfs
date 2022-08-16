#pragma once
#include <cstddef>

namespace zvfs
{
	class vfs_settings
	{
	public:
		/**
		* Creates a new settings object which can be used on multiple vfs instances
		* 
		* @param[in] lowercase_filesystem	Defines if the filesystem should convert every input to lowercase,
											also causes it to ignore double registrations if both evaluate to lowercase
		* @param[in] ansi_paths				Require paths to be in the ASCII range.
		*									Warning: Unicode encoded paths are not yet supported. Disable this with care!
		* @param[in] max_path				The maximum length of any provided path for a node
		* 
		* @exceptsafe no-throw
		*/
		[[nodiscard]] vfs_settings(bool lowercase_filesystem = true, bool ansi_paths = true, size_t max_path = 255);

		bool m_lowercase_filesystem;
		bool m_ansi_paths;
		size_t m_max_path;
	};

	namespace settings
	{
		extern vfs_settings g_default_settings;
	}
}