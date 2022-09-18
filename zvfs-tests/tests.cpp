#include "doctest.h"
#include <zvfs>

DOCTEST_TEST_CASE("vfs creation")
{
	std::vector<std::string> simulated_fs = {
		"file1.png",
		"folder1/",
		"folder2/",
		"folder2/file2.png",
		"folder2/file2.png",
		"file3"
	};

	// Our tests depend on this dataset matching these criteria
	//
	CHECK(simulated_fs.size() == 6);
	CHECK(simulated_fs[3] == simulated_fs[4]);

	zvfs::vfs* vfs = new zvfs::vfs(zvfs::settings::g_default_settings);

	for (auto& it : simulated_fs)
	{
		zvfs::node_data** entry = vfs->add(it);
		CHECK(entry != nullptr);

		bool is_folder = (it.rfind("/") == it.size() - 1);
		if (!is_folder)
			*entry = new zvfs::file();
	}

	// vfs is actually + 1 because of the root node
	// simulated_fs is - 1 because of the duplicate entry
	// so if both match, everything is good
	//
	CHECK(vfs->size() == simulated_fs.size());

	zvfs::node* current_folder = vfs->get("");
	CHECK(current_folder);
	CHECK(!current_folder->m_is_file);
	CHECK(current_folder->m_is_root);
	CHECK(current_folder->m_dir);

	size_t counter = 0;
	for(auto it : *current_folder->m_dir)
	{
		CHECK(it->parent() == current_folder);

		//"file1.png"
		//
		if(it->path() == simulated_fs[0])
		{
			CHECK(it->m_is_file);
			CHECK(it->m_file);
		}

		//"folder1/"
		//
		if (it->path() == simulated_fs[1])
		{
			CHECK(!it->m_is_file);

			// This folder is empty, there should be no folder data allocated for this
			//
			CHECK(!it->m_dir);
		}

		//"folder2/"
		//
		if (it->path() == simulated_fs[2])
		{
			CHECK(!it->m_is_file);
			CHECK(it->m_dir);

			CHECK(it->m_dir->size() == 1);

			auto child = *it->m_dir->begin();
			CHECK(child->parent() == it);
			CHECK((bool)(child->path() == simulated_fs[3]));
		}

		//"file3.png"
		//
		if (it->path() == simulated_fs[5])
		{
			CHECK(it->m_is_file);
			CHECK(it->m_file);
		}
		counter++;
	}

	//file1, file3, folder1, folder2
	//
	CHECK(counter == 4);

	delete vfs;
}

DOCTEST_TEST_CASE("vfs deletion")
{
	zvfs::vfs* vfs = new zvfs::vfs(zvfs::settings::g_default_settings);

	auto node_data = vfs->add("test");
	CHECK(node_data != nullptr);
	CHECK(vfs->size() == 2);
	CHECK(vfs->get("") != nullptr);

	// This should never be done in production code!
	//
	vfs->~vfs();
	
	node_data = vfs->add("test2");
	CHECK(node_data == nullptr);
	CHECK(vfs->size() == 0);
	CHECK(vfs->get("") == nullptr);
}

bool g_has_destructor_been_called_at_least_once = false;
class test_file : public zvfs::file
{
public:
	test_file()
		: a_file_property(123)
	{

	}

	virtual ~test_file()
	{
		CHECK(a_file_property == 123);

		g_has_destructor_been_called_at_least_once = true;
	}

private:
	int32_t a_file_property;

};

DOCTEST_TEST_CASE("vfs node insertion and deletion")
{
	std::vector<std::string> simulated_fs = {
		"file1.png",
		"folder1/",
		"folder2/",
		"folder2/file2.png",
		"folder2/file2.png",
		"folder2/file3.png",
		"folder2/file4.png",
		"folder2/folder3/file5.png",
		"folder2/folder3/folder4/file6.png",
		"file7"
	};

	// Our tests depend on this dataset matching these criteria
	//
	CHECK(simulated_fs.size() == 10);
	CHECK(simulated_fs[3] == simulated_fs[4]);

	zvfs::vfs* vfs = new zvfs::vfs(zvfs::settings::g_default_settings);

	for (auto& it : simulated_fs)
	{
		zvfs::node_data** entry = vfs->add(it);
		CHECK(entry != nullptr);

		bool is_folder = (it.rfind("/") == it.size() - 1);
		if (!is_folder)
		{
			*entry = new test_file();
		}
	}

	// Removing a file with recursive flag should fail
	//
	CHECK(vfs->remove("file1.png", true) == false);

	// Removing a file should work and decrement the current node count
	//
	auto size_before = vfs->size();
	CHECK(vfs->remove("file1.png") == true);
	CHECK(vfs->size() < size_before);

	// Removing a folder with children should fail without the recursive flag set
	//
	CHECK(vfs->remove("folder2/") == false);

	// Delete a single file and check that the node count only dropped by one
	//
	size_before = vfs->size();
	CHECK(vfs->remove("folder2/file3.png") == true);
	CHECK((size_before - vfs->size()) == 1);

	// Verify that the hierachy is still intact after the single deletion
	//
	auto node = vfs->get("folder2/folder3/folder4/file6.png");

	// This check verifies the following structure
	// 
	// [file6.png][folder4/][folder3/][folder2/][root]
	//
	CHECK(node != nullptr);
	CHECK(bool(node->path() == "folder2/folder3/folder4/file6.png"));

	CHECK(node->parent() != nullptr);
	CHECK(bool(node->parent()->path() == "folder2/folder3/folder4/"));

	CHECK(node->parent()->parent() != nullptr);
	CHECK(bool(node->parent()->parent()->path() == "folder2/folder3/"));

	CHECK(node->parent()->parent()->parent() != nullptr);
	CHECK(bool(node->parent()->parent()->parent()->path() == "folder2/"));

	CHECK(node->parent()->parent()->parent()->parent() != nullptr);
	CHECK(bool(node->parent()->parent()->parent()->parent()->path() == ""));
	CHECK(node->parent()->parent()->parent()->parent()->m_is_root == true);

	// This should decrement the node count by 7
	//
	size_before = vfs->size();
	CHECK(vfs->remove("folder2/", true) == true);
	CHECK((size_before - vfs->size()) == 7);

	// Check if the virtual destructor of our special file implementation is executed
	//
	CHECK(g_has_destructor_been_called_at_least_once == true);

	g_has_destructor_been_called_at_least_once = false;

	// This also triggers the delete routine on all nodes
	//
	delete vfs;

	// Check that the nodes got destructed
	//
	CHECK(g_has_destructor_been_called_at_least_once == true);
}

DOCTEST_TEST_CASE("vfs node insertion of illegal nodes")
{
	zvfs::vfs* vfs = new zvfs::vfs(zvfs::settings::g_default_settings);

	// Try to insert a invalid path
	//
	std::wstring unicode_invalid(L"\u7FFFFFFF");
	#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable: 4244)
	#endif
	std::string illegal_conversion(unicode_invalid.begin(), unicode_invalid.end());
	#ifdef _MSC_VER
	#pragma warning(pop)
	#endif

	CHECK(vfs->add(illegal_conversion) == nullptr);

	// Add a valid file
	//
	CHECK(vfs->add("file1.png") != nullptr);

	// Try to remove a non existing, but potentially valid file
	//
	CHECK(vfs->remove("file2.png") == false);

	// Try to remove an invalid path
	//
	CHECK(vfs->remove(illegal_conversion) == false);

	delete vfs;
}

DOCTEST_TEST_CASE("vfs recursive find")
{
	std::vector<std::string> simulated_fs = {
		"file1.png",
		"folder1/",
		"folder2/",
		"folder2/file2.png",
		"folder2/file2.png",
		"folder2/file3.png",
		"folder2/file4.txt",
		"folder2/folder3/file5.png",
		"folder2/folder3/folder4/file6.png",
		"folder2/folder3/folder4/file6.txt",
		"folder2/folder3/folder4txt/file7.txt",
		"folder2/folder3/folder4txt/file8.png",
		"file9"
	};

	zvfs::vfs* vfs = new zvfs::vfs(zvfs::settings::g_default_settings);

	// Prepare the dummy fs
	//
	for (auto& it : simulated_fs)
	{
		zvfs::node_data** entry = vfs->add(it);
		CHECK(entry != nullptr);

		bool is_folder = (it.rfind("/") == it.size() - 1);
		if (!is_folder)
		{
			*entry = new test_file();
		}
	}

	std::vector<zvfs::node*> nodes;

	// Find should return the amount stored in the out vector
	//
	CHECK(vfs->find(".txt", nodes) == nodes.size());

	// The amount of nodes in the out vector should match our dummy filesystem
	//
	CHECK(nodes.size() == 3);

	delete vfs;
}