#include "vfs.hpp"
#include "path.hpp"
#include <stdexcept>

namespace zvfs
{
	/**
	* File reader implementation
	*
	* @param[in] settings	You can either create your own zvfs::vfs_settings object or pass
	*						a default one from zvfs::settings
	* @exceptsafe no-throw
	*/
	vfs::vfs(vfs_settings& settings)
		: m_hasher()
		, m_settings(settings)
	{
		std::string_view empty_view;
		this->m_root_node = new node(false, true, m_hasher(""), empty_view);
		this->m_nodes.insert({ m_hasher(""), this->m_root_node });
		this->m_initialized = true;
	}

	/**
	* Performs a full cleanup on all linked nodes
	* Will invoke destructors of user defined zvfs::file overloads
	*
	* @exceptsafe no-throw
	*/
	vfs::~vfs()
	{
		this->remove_node(this->m_root_node, true);
		this->m_root_node = nullptr;

		this->m_initialized = false;
	}

	/**
	* Adds a new node to the vfs. Expects complete paths
	*
	* @param[in] path		Complete path to the added node
	*						Example: folder1/folder2/file.png
	* 
	* @returns				If the function succeeded it returns a pointer to a zvfs::node_data pointer
	*						that can be dereferenced and initialized with a custom file_data overload
	*
	*						Returns a nullptr on failure
	* @exceptsafe no-throw
	*/
	node_data** vfs::add(std::string_view path)
	{
		if (!this->m_initialized)
			return nullptr;

		node* entry = add_node(path);
		if (!entry)
			return nullptr;

		return entry->m_is_file ? reinterpret_cast<node_data**>(&entry->m_file) : reinterpret_cast<node_data**>(&entry->m_dir);
	}

	/**
	* Adds a new node to the vfs. Expects complete paths
	*
	* @overload
	*/
	node_data** vfs::add(std::string& path)
	{
		if (!this->m_initialized)
			return nullptr;

		node* entry = add_node(path);
		if (!entry)
			return nullptr;

		return entry->m_is_file ? reinterpret_cast<node_data**>(&entry->m_file) : reinterpret_cast<node_data**>(&entry->m_dir);
	}

	/**
	* Removes a node from the vfs. Expects complete paths
	*
	* @param[in] path		Complete path to the node
	*						Example: folder1/folder2/file.png
	* 
	* @param[in] recursive	Deletes multiple levels of sub-folders
	*						including any file present in those
	*
	*						Warning: The function will fail if specified with a path to a file
	* 
	* @returns				Returns true on successfull deletion
	*
	* @exceptsafe basic
	* @throws std::runtime_exception	Thrown if deletion corrupted the file hierachy or
	*									was unable to delete all nodes comletely
	*/
	bool vfs::remove(std::string_view path, bool recursive)
	{
		if (!this->m_initialized)
			return false;

		node* entry = this->get(path);
		if (!entry)
			return false;

		return this->remove_node(entry, recursive);
	}

	/**
	* Removes a node from the vfs. Expects complete paths
	*
	* @overload
	*/
	bool vfs::remove(std::string& path, bool recursive)
	{
		if (!this->m_initialized)
			return false;

		node* entry = this->get(path);
		if (!entry)
			return false;

		return this->remove_node(entry, recursive);
	}

	/**
	* Retrieves a node from the vfs. Expects complete paths
	*
	* @param[in] path		Complete path to the node
	*						Example: folder1/folder2/file.png
	* 
	* @returns				If the function succeeded it returns a pointer to the requested zvfs::node
	*						Returns a nullptr if node could not be found
	* @exceptsafe no-throw
	*/
	node* vfs::get(std::string_view path)
	{
		if (!this->m_initialized)
			return nullptr;

		size_t hash = this->m_hasher(path);
		return get_node(hash);
	}

	/**
	* Retrieves a node from the vfs. Expects complete paths
	*
	* @overload
	*/
	node* vfs::get(std::string& path)
	{
		if (!this->m_initialized)
			return nullptr;

		size_t hash = this->m_hasher(path);
		return get_node(hash);
	}

	/**
	* Retrieves the current settings of this instance
	* 
	* @returns				A pointer to the internal zvfs::vfs_settings object
	* @exceptsafe no-throw
	*/
	vfs_settings* vfs::get_settings()
	{
		return &this->m_settings;
	}

	/**
	* Retrieves the number of nodes linked in this instance
	* 
	* @returns				Number of nodes
	* @exceptsafe no-throw
	*/
	size_t vfs::size()
	{
		return this->m_nodes.size();
	}

	node* vfs::add_node(std::string_view path)
	{
		size_t hash = this->hash_entry(path);
		if (hash == static_cast<size_t>(-1))
			return nullptr;

		// The root node for each VFS is created while constructing the VFS itself
		//
		bool isroot = !static_cast<bool>(path.size());
		if (isroot)
			return this->m_root_node;

		// Check if the node already exists 
		//
		if (node* entry = get_node(hash))
			return entry;

		// Split the path into two seperate stringviews
		// First contains the path including trailing slash, second contains the filename (possibly including extension)
		//
		auto [split_path, split_file] = path::split_path(path);

		node* parent = add_node(split_path);

		bool isfile = !static_cast<bool>(path.back() == '/');
		node* entry = new node(isfile, false, hash, path);

		entry->set_parent(parent);

		if (!parent->m_dir)
		{
			parent->m_dir = new dir();
		}

		parent->m_dir->add_child(entry);


		this->m_nodes.insert({ hash, entry });

		return entry;
	}

	bool vfs::remove_node(node* entry, bool recursive)
	{
		auto delete_node = [this](node* entry) -> bool
		{
			if (!entry)
				return false;

			// Find the node in the root container
			//
			auto map_entry = this->m_nodes.find(entry->hash());
			if (map_entry == this->m_nodes.end())
				return false;

			// Remove it from the root container
			//
			this->m_nodes.erase(map_entry);

			// Verify that the parent hierachy is not corrupted
			// The root node is allowed to have no parent
			//
			if ((!entry->parent() && !entry->m_is_root) || (entry->parent() && entry->parent()->m_is_file))
				throw std::runtime_error("No parent, or corrupt parent");

			// Remove the node from its parents children list
			//
			if (entry->parent() && entry->parent()->m_dir)
			{
				if (!entry->parent()->m_dir->remove_child(entry))
					throw std::runtime_error("Failed to remove parrent, hierachy is likely corrupted");
			}

			// Perform actual deletion on the node object
			//
			delete entry;

			return true;
		};

		auto delete_recursive = [&delete_node](dir* folder, auto& self) -> bool
		{
			if (!folder)
				return false;

			std::vector<node*> temporary_remove_storage;
			temporary_remove_storage.reserve(folder->size());

			for (auto it : *folder)
			{
				// Handle recursive folders
				//
				if (!it->m_is_file)
				{
					// Check if the child folder has children of its own
					// If no file was ever added to a folder, the zvfs::dir ptr might be invalid
					//
					if (it->m_dir && it->m_dir->size())
					{
						// Propagate error upwards
						//
						if (!self(it->m_dir, self))
							return false;
					}
				}

				// Enqueue nodes that should be deleted
				// We cannot delete directly while iterating without doing complex iterator preservation
				//
				temporary_remove_storage.push_back(it);
			}

			// Delete all enqueued nodes
			//
			for (auto it : temporary_remove_storage)
			{
				// Deletion will propagate errors upwards 
				//
				if (!delete_node(it))
					return false;
			}

			// Assert a failure case that should never happen
			//
			if (folder->size())
				throw std::runtime_error("Folder still has children after recursive delete");

			return true;
		};

		if (entry->m_is_file)
		{
			// Cannot recursively delete files
			//
			if (recursive)
				return false;
		}
		else
		{
			// Check that the folder is empty if its not a recursive delete
			//
			if (!recursive && entry->m_dir && entry->m_dir->size())
				return false;

			// Perform recursive delete on child nodes
			//
			if (!delete_recursive(entry->m_dir, delete_recursive))
				return false;
		}

		return delete_node(entry);
	}

	node* vfs::get_node(size_t hash)
	{
		auto entry = this->m_nodes.find(hash);
		if (entry == this->m_nodes.end())
			return nullptr;

		return entry->second;
	}

	size_t vfs::hash_entry(std::string_view path)
	{
		// If the vfs is running in ansi path mode we need to verify
		// that all paths are legal. Only allow sensible inputs
		//
		if (this->m_settings.m_ansi_paths)
		{
			for (auto c : path)
			{
				if (c < 32 || c > 126)
					return static_cast<size_t>(-1);
			}
		}

		// Lowecase mode instructs the vfs to treat all inputs as lowercase paths
		// This will cause collisions if it doesn't match the source filesystems rules
		//
		if (this->m_settings.m_lowercase_filesystem)
		{
			std::string lower_case_path;
			lower_case_path.resize(path.size());

			std::transform(path.begin(), path.end(), lower_case_path.begin(), [](unsigned char c)
			{
				return static_cast<char>(std::tolower(c));
			});

			return this->m_hasher(lower_case_path.c_str());
		}
		return this->m_hasher(path.data());
	}
}