# zvfs ![example workflow](https://github.com/z3t4s/zvfs/actions/workflows/cmake.yml/badge.svg)
Platform agnostic virtual file system


- Leightweight
- Doxygen compatible
- Mostly exception safe, documented where it is not the case
- Easy to integrate
- Actual useful test coverage

# Example
Checkout [example.cpp](https://github.com/z3t4s/zvfs/blob/main/zvfs-tests/example.cpp) for the full .tar parsing code

```c++
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

void example()
{
  auto file = vfs_one->add(std::string_view(header.name));
  auto filestream = new sub_stream(stream, stream->tellg(), size);
	*file = new tar_file(filestream);
}
```
