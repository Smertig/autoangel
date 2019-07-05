//
// Created by int3 on 21.02.19.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <fstream>

namespace pck {

class package {
	std::string path;

	struct key_header_t {
		uint32_t key1;
		uint32_t start_offset;
		uint32_t key2;
	};
	static_assert(sizeof(key_header_t) == 12, "size mismatch");

	struct package_header_t {
		uint32_t guard0;
		uint32_t version;
		uint32_t entry_offset;
		uint32_t flags;
		char description[252];
		uint32_t guard1;
	};
	static_assert(sizeof(package_header_t) == 0x110, "size mismatch");

	struct file_gbk_entry_t {
		char filename[260];
		uint32_t offset;
		uint32_t size;
		uint32_t compressed_size;
		uint32_t dummy;
	};
	static_assert(sizeof(file_gbk_entry_t) == 0x114, "size mismatch");

	struct file_entry_t {
		std::u16string filename;
		uint32_t offset, size, compressed_size;
	};

	struct comparator;

	std::ifstream ifs;
	key_header_t key_header;
	package_header_t package_header;
	uint32_t total_size;
	std::vector<file_entry_t> file_entries;

	void read_key_header();
public:
	explicit package(std::string path);

	void load();
	std::vector<uint8_t> read(std::u16string path);
	std::pair<std::vector<file_entry_t>::iterator, std::vector<file_entry_t>::iterator> find_prefix(std::u16string prefix);
};

}