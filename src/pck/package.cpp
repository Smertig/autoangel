//
// Created by int3 on 21.02.19.
//

#include "package.h"

#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>
#include <cassert>
#include <memory>
#include <algorithm>
#include <src/util/encoding.h>

#include <fmt/format.h>

#include <zlib.h>

static constexpr const uint32_t xor_keys[4] = {
	0xFDFDFEEE,
	0xF00DBEEF,
	0xA8937462,
	0x59374231
};

void read_some(std::istream& is, void* out, size_t size) {
	auto pos = is.tellg();
	bool res = !!is.read(reinterpret_cast<char*>(out), size);
	if (!res) {
		throw std::runtime_error(fmt::format("cannot read {} bytes at pos {}", size, static_cast<int>(pos)));
	}
}

template <class T>
void read_some(std::istream& is, T& out) {
	read_some(is, &out, sizeof(T));
}

void decompress(const void* input, std::size_t input_size, void* output, std::size_t* output_size) {
	uLongf output_size2 = *output_size;
	auto res = uncompress(reinterpret_cast<Bytef*>(output), &output_size2, reinterpret_cast<const Bytef*>(input), static_cast<uLongf>(input_size));
	if (res != Z_OK) {
		throw std::runtime_error(fmt::format("uncompress fails: {}", zError(res)));
	}
	*output_size = static_cast<std::size_t>(output_size2);
}

template <class ForwardIt, class T, class Compare=std::less<>>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare comp = {}) {
	first = std::lower_bound(first, last, value, comp);
	return first != last && !comp(value, *first) ? first : last;
}

namespace pck {

void package::read_key_header() {
	ifs.seekg(0, std::istream::beg);

	read_some(ifs, this->key_header);

	constexpr uint32_t expected_key1 = 0x4DCA23EF;
	constexpr uint32_t expected_key2 = 0x56A089B7;
	if (this->key_header.key1 != expected_key1) throw std::runtime_error(fmt::format("invalid key1 {:08X}, expected {:08X}", this->key_header.key1, expected_key1));
	if (this->key_header.key2 != expected_key2) throw std::runtime_error(fmt::format("invalid key2 {:08X}, expected {:08X}", this->key_header.key2, expected_key2));

	ifs.seekg(0, std::istream::beg);
}

package::package(std::string path_) : path(std::move(path_)) {

}

void package::load() {
	ifs = std::ifstream(path, std::ifstream::binary);

	if (!ifs) {
		throw std::runtime_error(fmt::format("failed to open '{}'", path));
	}

	this->read_key_header();

	ifs.seekg(0, std::istream::end);

	auto size = ifs.tellg();
	if (size > std::numeric_limits<uint32_t>::max()) {
		throw std::runtime_error("too big file");
	}

	this->total_size = static_cast<uint32_t>(size);
	auto start_offset = this->key_header.start_offset;

	ifs.seekg(start_offset - 4, std::ifstream::beg);
	uint32_t version;
	read_some(ifs, version);

	if (version != 0x20002 && version != 0x20001) {
		throw std::runtime_error("incorrect version");
	}

	ifs.seekg(start_offset - 8, std::ifstream::beg);
	uint32_t file_num;
	read_some(ifs, file_num);

	ifs.seekg(start_offset - 8 - sizeof(package_header_t), std::ifstream::beg);
	read_some(ifs, this->package_header);

	if (std::strstr(this->package_header.description, "lica File Package") == nullptr) {
		throw std::runtime_error("unknown description");
	}

	strncpy(this->package_header.description,
			"Angelica File Package, Perfect World Co. Ltd. 2002~2008. All Rights Reserved. ",
			sizeof(this->package_header.description)
	);

	bool pack_encrypted = (this->package_header.flags & 0x8000'0000) != 0;
	assert(!pack_encrypted);

	this->package_header.entry_offset ^= xor_keys[2];
	if (this->package_header.guard0 != xor_keys[0]) throw std::runtime_error("invalid guard0");
	if (this->package_header.guard1 != xor_keys[1]) throw std::runtime_error("invalid guard1");

	ifs.seekg(this->package_header.entry_offset, std::ifstream::beg);
	this->file_entries.reserve(file_num);

	for (uint32_t i = 0; i < file_num; i++) {
		this->file_entries.emplace_back();
		auto& entry = this->file_entries.back();

		uint32_t compressed_size;
		read_some(ifs, compressed_size);
		compressed_size ^= xor_keys[2];

		uint32_t compressed_size2;
		read_some(ifs, compressed_size2);
		compressed_size2 ^= xor_keys[2] ^ xor_keys[3];

		if (compressed_size != compressed_size2) {
			throw std::runtime_error("check byte error");
		}

		auto entry_bytes = std::make_unique<uint8_t[]>(compressed_size);
		read_some(ifs, entry_bytes.get(), compressed_size);

		std::size_t entry_size = sizeof(file_gbk_entry_t);
		if (entry_size == compressed_size) {
			// assuming, it's compressed
			assert(false);
		}
		else {
			file_gbk_entry_t gbk_entry;
			decompress(entry_bytes.get(), compressed_size, &gbk_entry, &entry_size);
			entry.filename = gbk_to_unicode(gbk_entry.filename);
			entry.offset = gbk_entry.offset;
			entry.size = gbk_entry.size;
			entry.compressed_size = gbk_entry.compressed_size;
		}
	}

	std::sort(file_entries.begin(), file_entries.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.filename < rhs.filename;
	});
}

struct package::comparator {
	bool operator()(const std::u16string& lhs, const file_entry_t& rhs) {
		return lhs < rhs.filename;
	}

	bool operator()(const file_entry_t& lhs, const std::u16string& rhs) {
		return lhs.filename < rhs;
	}
};

std::vector<uint8_t> package::read(std::u16string path) {
	std::replace(path.begin(), path.end(), L'/', L'\\');
	std::transform(path.begin(), path.end(), path.begin(), [](char16_t c) -> char16_t {
		return std::towlower(c);
	});

	auto it = binary_find(file_entries.begin(), file_entries.end(), path, comparator{});
	if (it == file_entries.end()) {
		return {};
	}

	ifs.seekg(it->offset, std::ifstream::beg);
	auto compressed = std::vector<uint8_t>(it->compressed_size);
	read_some(ifs, compressed.data(), compressed.size());

	auto decompressed = std::vector<uint8_t>(it->size);
	std::size_t output_size = decompressed.size();
	decompress(compressed.data(), compressed.size(), decompressed.data(), &output_size);
	assert(output_size == decompressed.size());

	return decompressed;
}

auto package::find_prefix(std::u16string prefix) -> std::pair<std::vector<file_entry_t>::iterator, std::vector<file_entry_t>::iterator> {
	std::replace(prefix.begin(), prefix.end(), L'/', L'\\');
	std::transform(prefix.begin(), prefix.end(), prefix.begin(), [](char16_t c) -> char16_t {
		return std::towlower(c);
	});

	auto first = std::lower_bound(file_entries.begin(), file_entries.end(), prefix, comparator{});
	auto last = first;

	while(last != file_entries.end() && last->filename.find(prefix) == 0) { ++last; }
	return { first, last };
}

}


