#include "node.hpp"
namespace zvfs
{
	/**
	* Retrieves a the nodes parent node
	* 
	* @returns				If the node has a parent this function returns the pointer to it
	*						Returns a nullptr if node has no parent (Only the case for the root node)
	* @exceptsafe no-throw
	*/
	node* node::parent()
	{
		return this->m_parent;
	}

	/**
	* Sets the nodes parent node
	* 
	* @param[in] parent		The parent node pointer
	*
	* @exceptsafe no-throw
	*/
	void node::set_parent(node* parent)
	{
		this->m_parent = parent;
	}

	/**
	* Retrieves the nodes full path
	*
	* @exceptsafe no-throw
	*/
	std::string_view node::path()
	{
		return this->m_path;
	}

	/**
	* Retrieves the node hash (hash of the full path)
	*
	* @exceptsafe no-throw
	*/
	size_t node::hash()
	{
		return this->m_hash;
	}

	node::node(bool is_file, bool is_root, size_t hash, std::string& path)
		: m_is_file(is_file)
		, m_is_root(is_root)
		, m_file(nullptr)
		, m_hash(hash)
		, m_path(path)
		, m_parent(nullptr)
	{
	}

	node::node(bool is_file, bool is_root, size_t hash, std::string_view path)
		: m_is_file(is_file)
		, m_is_root(is_root)
		, m_file(nullptr)
		, m_hash(hash)
		, m_path(path)
		, m_parent(nullptr)
	{
	}

	node::~node()
	{
		if (m_is_file)
			delete this->m_file;
		else
			delete this->m_dir;

		this->m_file = nullptr;
	}

	/**
	* Retrieves the begin Iterator used to iterate over children nodes
	*
	* @returns				A std::vector<node*> iterator to the begin of the container
	*
	* @exceptsafe no-throw
	*/
	std::vector<node*>::iterator dir::begin()
	{
		return this->m_children.begin();
	}

	/**
	* Retrieves the end Iterator used to iterate over children nodes
	*
	* @returns				A std::vector<node*> iterator to the end of the container
	*
	* @exceptsafe no-throw
	*/
	std::vector<node*>::iterator dir::end()
	{
		return this->m_children.end();
	}

	/**
	* Add a children node to this directory
	*
	* @param[in] entry		The child node to add
	*
	* @exceptsafe no-throw
	*/
	void dir::add_child(node* entry)
	{
		this->m_children.push_back(entry);
	}

	/**
	* Remove a children node from this directory
	*
	* @param[in] entry		The child node to remove
	* @returns				Returns true on success
	*
	* @exceptsafe no-throw
	*/
	bool dir::remove_child(node* entry)
	{
		auto result = std::find_if(this->m_children.begin(), this->m_children.end(), [entry](node* it)
		{
			return it == entry;
		});

		if (result == this->m_children.end())
			return false;

		this->m_children.erase(result);

		return true;
	}

	/**
	* Returns the number of child nodes in this directory
	*
	* @returns				Number of child nodes
	*
	* @exceptsafe no-throw
	*/
	size_t dir::size()
	{
		return this->m_children.size();
	}
}