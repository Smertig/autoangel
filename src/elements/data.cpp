//
// Created by int3 on 14.02.19.
//

#include "data.h"
#include "config.h"

#include <fmt/format.h>

#include <fstream>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace elements {

void data::parse(std::ifstream& ifs, const std::vector<config::ptr>& confs) {
	auto read_some = [&](auto& out) {
		ifs.read(reinterpret_cast<char*>(&out), sizeof(out));
	};

	uint16_t version{}, unknown{};

	read_some(version);
	read_some(unknown);

	auto it = std::find_if(confs.begin(), confs.end(), [version](auto&& conf) {
		return conf->version() == version;
	});

	if (it == confs.end()) {
		throw std::runtime_error("no config for version " + std::to_string(version));
	}
	this->_config = *it;
	auto& conf = *_config;

	_lists.reserve(conf.size());

	for (auto& list : conf.get_lists()) {
		const auto prefix_len = [&]() -> std::size_t {
			if (list->offset != static_cast<std::size_t>(-1)) {
				return list->offset;
			}
			else {
				if (list->caption == "021 - SKILLTOME_SUB_TYPE") {
					uint32_t tag{}, len{};
					read_some(tag);
					read_some(len);
					ifs.seekg(-8, ifs.cur);
					return 8 + len + 4;
				}
				else if (list->caption == "101 - NPC_WAR_TOWERBUILD_SERVICE") {
					uint32_t tag{}, len{};
					read_some(tag);
					read_some(len);
					ifs.seekg(-8, ifs.cur);
					return 8 + len;
				}
				else {
					throw std::runtime_error("unknown structure of '" + list->caption + "' prefix");
				}
			}
		}();
		std::vector<char> prefix(prefix_len);
		ifs.read(prefix.data(), prefix.size());

		uint32_t length{};
		read_some(length);

		_lists.emplace_back(std::make_shared<data_list>());
		auto& values_list = *_lists.back();

		values_list.type = list->dt;
		values_list.space = list->space;
		values_list.prefix = std::move(prefix);
		values_list.storage.reserve(length);

		for (uint32_t i = 0; i < length; i++) {
			values_list.storage.emplace_back(std::make_shared<data_value>());
			auto& value = values_list.storage.back();

			for (auto& meta_field : list->fields) {
				const auto field_size = meta_field.second.size(ifs);
				std::vector<char> raw_buffer(field_size);
				ifs.read(raw_buffer.data(), field_size);

				auto& field = value->value[meta_field.first];
				field.vtable = &meta_field.second;
				field.value = std::move(raw_buffer);
			}
		}
	}
}

data::data(const char* path) : _path(path) {
	std::ifstream ifs(path, std::ifstream::binary);
	if (!ifs) {
		throw std::runtime_error(fmt::format("cannot open elements.data at '{}'", _path));
	}

	if (!ifs.read(reinterpret_cast<char*>(&_version), sizeof(_version))) {
		throw std::runtime_error(fmt::format("cannot read version from first 2 bytes of '{}'", _path));
	}
}

void data::load(config::ptr conf) {
	std::vector<config::ptr> confs;
	confs.emplace_back(std::move(conf));
	load(confs);
}

void data::load(const std::vector<std::shared_ptr<config>>& confs) {
	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	ifs.open(_path, std::ifstream::binary);

	this->parse(ifs, confs);
}

void data::save(const char* path) {
	if (!path) {
		path = _path.c_str();
	}

	auto& conf = _config;

	std::ofstream ofs;
	ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

	ofs.open(path, std::ofstream::binary);

	auto write_some = [&](const auto& out) {
		ofs.write(reinterpret_cast<const char*>(&out), sizeof(out));
	};

	write_some(conf->version());
	write_some(uint16_t(0x3000));

	size_t list_index = 0;
	for (auto& list : conf->get_lists()) {
		auto& values_list = *_lists[list_index++];

		assert(list->offset == static_cast<std::size_t>(-1) || list->offset == values_list.prefix.size());

		ofs.write(values_list.prefix.data(), values_list.prefix.size());

		auto length = static_cast<uint32_t>(values_list.size());
		write_some(length);

		for (auto& value : values_list) {
			for (auto& meta_field : list->fields) {
				auto& field = value->value[meta_field.first];

				ofs.write(field.value.data(), field.value.size());
			}
		}
	}
}

}