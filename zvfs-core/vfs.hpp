#pragma once
#include "settings.hpp"
#include "node.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace zvfs
{
	/**
	* The base class of this library
	*
	* It defines the root object representing one instance of a virtual file system
	*/
	class vfs
	{
	public:
		/**
		* File reader implementation
		*
		* @param[in] settings	You can either create your own zvfs::vfs_settings object or pass
		*						a default one from zvfs::settings
		* @exceptsafe no-throw
		*/
		[[nodiscard]] vfs(vfs_settings& settings = settings::g_default_settings);

		/**
		* Performs a full cleanup on all linked nodes
		* Will invoke destructors of user defined zvfs::file overloads
		*
		* @exceptsafe no-throw
		*/
		~vfs();

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
		[[nodiscard]] node_data** add(std::string_view path);

		/**
		* Adds a new node to the vfs. Expects complete paths
		*
		* @overload
		*/
		[[nodiscard]] node_data** add(std::string& path);

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
		[[nodiscard]] bool remove(std::string_view path, bool recursive = false);

		/**
		* Removes a node from the vfs. Expects complete paths
		*
		* @overload
		*/
		[[nodiscard]] bool remove(std::string& path, bool recursive = false);

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
		[[nodiscard]] node* get(std::string_view path);

		/**
		* Retrieves a node from the vfs. Expects complete paths
		*
		* @overload
		*/
		[[nodiscard]] node* get(std::string& path);

		/**
		* Retrieves a list of nodes matching a query string on the path
		*
		* @param[in] filter		Substring of the node path
		*						Example: ".txt", "file.extension" or "folder1/file.png"
		*
		* @returns				If the function succeeded it returns the number of found nodes matching the query
		*						Returns -1 if the vfs couldn't be searched
		* @exceptsafe no-throw
		*/
		[[nodiscard]] size_t find(std::string_view filter, std::vector<node*>& out_nodes);

		/**
		* Retrieves a list of nodes matching a query string on the path
		*
		* @overload
		*/
		[[nodiscard]] size_t find(std::string& filter, std::vector<node*>& out_nodes);

		/**
		* Retrieves the current settings of this instance
		* 
		* @returns				A pointer to the internal zvfs::vfs_settings object
		* @exceptsafe no-throw
		*/
		[[nodiscard]] vfs_settings* get_settings();

		/**
		* Retrieves the number of nodes linked in this instance
		* 
		* @returns				Number of nodes
		* @exceptsafe no-throw
		*/
		[[nodiscard]] size_t size();

	private:
		node* add_node(std::string_view path);
		bool remove_node(node* entry, bool recursive);
		node* get_node(size_t hash);
		size_t hash_entry(std::string_view path);

	private:
		std::hash<std::string_view> m_hasher;
		std::unordered_map<size_t, node*> m_nodes;
		vfs_settings m_settings;
		node* m_root_node;
		bool m_initialized;
	};
}