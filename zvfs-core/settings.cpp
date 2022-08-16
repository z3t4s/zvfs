#include "settings.hpp"

namespace zvfs
{
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
	vfs_settings::vfs_settings(bool lowercase_filesystem, bool ansi_paths, size_t max_path)
		: m_lowercase_filesystem(lowercase_filesystem)		
		, m_ansi_paths(ansi_paths)
		, m_max_path(max_path)
	{

	}

	namespace settings
	{
		vfs_settings g_default_settings(true, true, 255);
	}
} // namespace zvfs