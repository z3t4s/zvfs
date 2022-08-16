#pragma once
#include <string>
#include <vector>

namespace zvfs
{
	class node;
	class node_data;
	class file;
	class dir;

	/**
	* This class represents one data point inside the vfs
	* It could either be a directory or file
	*
	*/
	class node
	{
		// Used to make the node constructor available in the vfs class
		//
		friend class vfs;

	public:
		/**
		* Retrieves a the nodes parent node
		* 
		* @returns				If the node has a parent this function returns the pointer to it
		*						Returns a nullptr if node has no parent (Only the case for the root node)
		* @exceptsafe no-throw
		*/
		[[nodiscard]] node* parent();

		/**
		* Sets the nodes parent node
		* 
		* @param[in] parent		The parent node pointer
		*
		* @exceptsafe no-throw
		*/
		void set_parent(node* parent);

		/**
		* Retrieves the nodes full path
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] std::string_view path();

		/**
		* Retrieves the node hash (hash of the full path)
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] size_t hash();

		bool m_is_file;
		bool m_is_root;

		union
		{
			file* m_file;
			dir* m_dir;
		};

	private:
		node(bool is_file, bool is_root, size_t hash, std::string& path);
		node(bool is_file, bool is_root, size_t hash, std::string_view path);
		~node();

	private:
		size_t m_hash;
		std::string m_path;
		node* m_parent;
	};

	/**
	* Base class for node data
	* Interface usage only
	*
	*/
	class node_data
	{
	protected:
		virtual ~node_data() = default;
	};

	/**
	* A node data container for files
	* Not useful in itself, provides a specialized type that can be overloaded further
	* See tests or example for reference
	*
	*/
	class file : public node_data
	{

	};

	/**
	* A node data container for directorys
	* This could be overloaded, but its not required for normal usage	
	*
	*/
	class dir : public node_data
	{
	public:
		/**
		* Retrieves the begin Iterator used to iterate over children nodes
		*
		* @returns				A std::vector<node*> iterator to the begin of the container
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] std::vector<node*>::iterator begin();

		/**
		* Retrieves the end Iterator used to iterate over children nodes
		*
		* @returns				A std::vector<node*> iterator to the end of the container
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] std::vector<node*>::iterator end();

		/**
		* Add a children node to this directory
		*
		* @param[in] entry		The child node to add
		*
		* @exceptsafe no-throw
		*/
		void add_child(node* entry);

		/**
		* Remove a children node from this directory
		*
		* @param[in] entry		The child node to remove
		* @returns				Returns true on success
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] bool remove_child(node* entry);

		/**
		* Returns the number of child nodes in this directory
		*
		* @returns				Number of child nodes
		*
		* @exceptsafe no-throw
		*/
		[[nodiscard]] size_t size();

	private:
		std::vector<node*> m_children;
	};
}