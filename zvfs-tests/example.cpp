#include <zvfs>
#include <stdint.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>

class dummy_stream
{
public:
	dummy_stream();
	dummy_stream(const std::string& file_path);
	~dummy_stream();

	[[nodiscard]] bool read(void* dst, size_t len);	
	[[nodiscard]] std::streampos tellg();
	[[nodiscard]] bool seekg(std::streamoff pos);
	[[nodiscard]] bool advance(size_t len);

	[[nodiscard]] size_t size();

	void reference();
	uint64_t release();

	dummy_stream(const dummy_stream&) = delete;
	dummy_stream& operator=(const dummy_stream&) = delete;
private:
	uint64_t m_refcount;
	std::fstream m_file_stream;
	std::streamoff m_current_offset;
	std::streampos m_current_filesize;
};

dummy_stream::dummy_stream()
	: m_refcount(1)
	, m_current_offset(0)
	, m_current_filesize(0)
{

}

dummy_stream::dummy_stream(const std::string& file_path)
	: dummy_stream()
	//: m_refcount(1)
	//, m_current_offset(0)
	//, m_current_filesize(0)
{
	this->m_file_stream.open(file_path.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!this->m_file_stream.is_open())
	{
		throw std::runtime_error("Could not open file");
		return;
	}

	this->m_file_stream.seekg(0, std::ios::end);
	this->m_current_filesize = this->m_file_stream.tellg();
	this->m_file_stream.seekg(0, std::ios::beg);
}

dummy_stream::~dummy_stream()
{
	this->m_file_stream.close();
}

bool dummy_stream::read(void* dst, size_t len)
{
	// Check that the destination memory is available
	//
	if (!dst)
		return false;

	// We define a zero length read as failed
	//
	if (len == 0)
		return false;

	// Check for out of bounds read
	//
	if (this->m_current_offset + len >= static_cast<size_t>(this->m_current_filesize))
		return false;

	this->m_file_stream.read(reinterpret_cast<char*>(dst), len);
	this->m_current_offset = this->m_file_stream.tellg();
	return true;
}

std::streampos dummy_stream::tellg()
{
	return this->m_current_offset;
}

bool dummy_stream::seekg(std::streamoff pos)
{
	// Check for out of bounds seek
	//
	if (pos >= this->m_current_filesize)
		return false;

	this->m_file_stream.seekg(pos);
	this->m_current_offset = this->m_file_stream.tellg();
	return true;
}

bool dummy_stream::advance(size_t len)
{
	// Check for out of bounds seek
	//
	if (this->m_current_offset + len >= static_cast<size_t>(this->m_current_filesize))
		throw std::runtime_error("Out of bounds seekg");

	this->m_file_stream.seekg(this->m_current_offset + len);
	this->m_current_offset = this->m_file_stream.tellg();
	return true;
}

size_t dummy_stream::size()
{
	return this->m_current_filesize;
}

void dummy_stream::reference()
{
	this->m_refcount++;
}

uint64_t dummy_stream::release()
{
	uint64_t refcount = this->m_refcount--;
	if (refcount == 0)
		delete this;

	return refcount;
}

class sub_stream : public dummy_stream
{
public:
	sub_stream(dummy_stream* root_stream, int64_t offset, int64_t size);
	~sub_stream();

	bool read(void* dst, size_t len);
	size_t size();

private:
	dummy_stream* m_root_stream;
	int64_t m_offset;
	int64_t m_size;
};

sub_stream::sub_stream(dummy_stream* root_stream, int64_t offset, int64_t size)
	: m_root_stream(root_stream)
	, m_offset(offset)
	, m_size(size)
{
	this->m_root_stream->reference();
}

sub_stream ::~sub_stream()
{
	this->m_root_stream->release();
}

bool sub_stream::read(void* dst, size_t len)
{
	if (m_offset + len > this->m_root_stream->size())
		return false;

	if (!this->m_root_stream->seekg(m_offset))
		return false;

	return this->m_root_stream->read(dst, len);
}

size_t sub_stream::size()
{
	return this->m_size;
}

class tar_file : public zvfs::file
{
public:
	tar_file(sub_stream* stream)
		: m_stream(stream)
	{
		this->m_stream->reference();
	};

	size_t size()
	{
		return m_stream->size();
	}

	bool read(std::vector<uint8_t>& dst)
	{
		dst.resize(this->size());
		return m_stream->read(&dst[0], this->size());
	}

private:
	sub_stream* m_stream;
};

template<typename T>
void formated_cout(T t, size_t width)
{
	std::cout << std::left << std::setw(width) << std::setfill('\0') << t;
}

#if 0
int main(int argc, char** argv)
#else
int excluded(char** argv)
#endif
{
	// Structure and helper function definitions for the example "arbitrary binary blob to VFS" parsing
	//
	struct posix_header
	{						/* byte offset */
		char name[100];		/*   0 */
		char mode[8];		/* 100 */
		char uid[8];		/* 108 */
		char gid[8];		/* 116 */
		char size[12];		/* 124 */
		char mtime[12];		/* 136 */
		char chksum[8];		/* 148 */
		char typeflag;		/* 156 */
		char linkname[100]; /* 157 */
		char magic[6];		/* 257 */
		char version[2];	/* 263 */
		char uname[32];		/* 265 */
		char gname[32];		/* 297 */
		char devmajor[8];	/* 329 */
		char devminor[8];	/* 337 */
		char prefix[155];	/* 345 */
							/* 500 */
	};

	auto octalstr_to_int32 = [](const char* str, uint32_t strlen)->auto
	{
		unsigned int output = 0;
		while (strlen > 0)
		{
			output = output * 8 + *str - '0';
			str++;
			strlen--;
		}
		return output;
	};
	
	auto round_up = [](unsigned n, unsigned incr) -> auto
	{
		return n + (incr - n % incr) % incr;
	};

	// Create a "agnostic" vfs
	//
	zvfs::vfs* vfs_one = new zvfs::vfs(zvfs::settings::g_default_settings);

	// Read current execution environment and open the binary test file
	//
	std::filesystem::path path(argv[0]);
	path = path.parent_path();
	path = path.parent_path();
	path = path.parent_path();
	path = path.parent_path() /= "test.tar";
	
	dummy_stream* stream = new dummy_stream(path.string());

	// Parse example .tar file
	//
	while (1)
	{
		posix_header header;
		if(!stream->read(&header, sizeof(posix_header)))
		{
			throw std::runtime_error("Failed to read .tar stream");
		}

		if (memcmp(header.magic, "ustar", sizeof(posix_header::magic)))
			break;

		if(!stream->advance(512 - sizeof(posix_header)))
			throw std::runtime_error("Out of bounds seek");

		int32_t size = 0;
		if (header.size != std::string_view("00000000000"))
		{
			size = octalstr_to_int32(header.size, sizeof(posix_header::size) - 1);
		}

		auto file = vfs_one->add(std::string_view(header.name));

		if (size)
		{
			if (header.typeflag != '0' && header.typeflag != 0)
				throw std::runtime_error("Should be a file");

			auto filestream = new sub_stream(stream, stream->tellg(), size);
			*file = new tar_file(filestream);
		}

		int32_t advance = round_up(size, 512);
		if(!stream->advance(advance))
			throw std::runtime_error("Out of bounds seek");
	}

	/* The structure should look like this
	* [root]
	*	- folder1
	*		- two.png
	*	- one.png
	*/
	auto split_command = [](std::string& command, std::vector<std::string>& segments)
	{
		std::stringstream ss(command);

		std::string segment;
		while (std::getline(ss, segment, ' '))
		{
			segments.push_back(segment);
		}
	};

	std::string command;
	command.reserve(10 + vfs_one->get_settings()->m_max_path);

	zvfs::node* current_folder = vfs_one->get("");
	while (true)
	{
		std::cout << current_folder->path() << ">";

		std::getline(std::cin, command);

		std::vector<std::string> split_commands;
		split_command(command, split_commands);

		if (split_commands[0] == "cd")
		{
			if (split_commands.size() == 1 || split_commands.size() > 2)
			{
				std::cout << "Invalid amount of commands. Expected 2: cd path/to/folder/" << std::endl;
				continue;
			}
				
			// Handle upward directory traversal
			//
			if (split_commands[1] == "..")
			{
				if (!current_folder->m_is_root)
					current_folder = current_folder->parent();
				continue;
			}

			// Handle trailing slash / no trailing slash
			//
			size_t trailing_slash_offset = split_commands[1].rfind('/');
			if (trailing_slash_offset != split_commands[1].size() -1)
			{
				split_commands[1] += '/';
				trailing_slash_offset = split_commands[1].size() - 1;
			}

			
			// Check if there is another slash in there, we don't support multi level traversal atm
			//
			size_t secondary_slash_offset = split_commands[1].rfind('/', trailing_slash_offset -1);
			if (secondary_slash_offset != static_cast<size_t>(-1))
			{
				std::cout << "Multilevel traversal is not supported yet" << std::endl;
				continue;
			}

			bool changed_dir = false;
			for (auto it : *current_folder->m_dir)
			{
				if (it->path() == split_commands[1])
				{
					if (it->m_is_file)
					{
						std::cout << "Specified file, expected directory" << std::endl;
						continue;
					}

					current_folder = it;
					changed_dir = true;
					break;
				}
			}

			if (!changed_dir)
				std::cout << "Directory not found. Try \"ls\"" << std::endl;
		}
		else if (split_commands[0] == "ls")
		{
			// Calculate the layout for the table
			//
			size_t longest_path = 0;
			for (auto it : *current_folder->m_dir)
			{
				if (longest_path < it->path().size())
				{
					auto [lhs, rhs] = zvfs::path::split_path(it->path());
					longest_path = rhs.size();
				}
			}

			// Print the table header
			//
			formated_cout("Type", 8);
			formated_cout("Path", longest_path + 8);
			formated_cout("Size", 8);
			std::cout << std::endl << std::endl;

			// Print the table data
			//
			for (auto it : *current_folder->m_dir)
			{
				if (it->m_is_file)
					formated_cout("[f]", 8);
				else
					formated_cout("[d]", 8);

				auto [lhs, rhs] = zvfs::path::split_path(it->path());
				formated_cout(rhs, longest_path + 8);

				if (it->m_is_file)
				{
					tar_file* file = (tar_file*)it->m_file;
					formated_cout((std::to_string(file->size() / 1024) + "Kib"), 8);
				}

				std::cout << std::endl;
			}
		}
		else if (split_commands[0] == "dump")
		{
			bool found_file = false;
			for (auto it : *current_folder->m_dir)
			{
				if (!it->m_is_file)
					continue;

				auto [lhs, rhs] = zvfs::path::split_path(it->path());
				if (rhs != split_commands[1])
					continue;

				std::vector<uint8_t> file_data;
				tar_file* file = (tar_file*)it->m_file;
				if (!file->read(file_data))
					throw std::runtime_error("Failed to read the file");

				std::ofstream outfile(rhs.data(), std::ofstream::binary | std::ofstream::trunc);
				if (!outfile.is_open())
					throw std::runtime_error("Failed to open destination file");

				outfile.write(reinterpret_cast<const char*>(&file_data[0]), file_data.size());
				outfile.close();
				found_file = true;
				break;
			}

			if (!found_file)
				std::cout << "File not found. Is it a directory?" << std::endl;
		}
		else
		{
			std::cout << "Unknown command. Try \"help\"" << std::endl;
		}
	}

	return 0;
}