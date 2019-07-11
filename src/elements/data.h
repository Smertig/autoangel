//
// Created by int3 on 14.02.19.
//

#pragma once

#include <fstream>
#include <vector>
#include <map>
#include <memory>

#include "config.h"

namespace elements {

class config;
struct meta_type;

struct field_value {
	meta_type* vtable = nullptr;
	std::vector<char> value;

	friend inline bool operator==(const field_value& lhs, const field_value& rhs) {
		return std::tie(lhs.vtable, lhs.value) == std::tie(rhs.vtable, rhs.value);
	}

	template <class F>
	void get(F&& callback) const {
		vtable->get(callback, value.data());
	}

	template <class F>
	void set(F&& callback) {
		vtable->set(callback, value.data());
	}
};

struct data_value {
	using storage_t = std::map<std::string, field_value>;

	storage_t value;

	friend inline bool operator==(const data_value& lhs, const data_value& rhs) {
		return lhs.value == rhs.value;
	}

	data_value clone() const {
		return data_value{ *this };
	}

	using ptr = std::shared_ptr<data_value>;

	//using value_type = storage_t::value_type;
	using iterator = storage_t::const_iterator;
	using const_iterator = storage_t::const_iterator;
	using size_type = storage_t::size_type;

	std::size_t size() const { return value.size(); }
	iterator begin() { return value.begin(); }
	iterator end() { return value.end(); }
	const_iterator begin() const { return value.begin(); }
	const_iterator end() const { return value.end(); }
};

struct data_list {
	using ptr = std::shared_ptr<data_list>;

	using storage_t = std::vector<data_value::ptr>;

	storage_t storage;
	data_type type;
	space_id space;
	std::vector<char> prefix;

	using value_type = storage_t::value_type;
	using iterator = storage_t::const_iterator;
	using const_iterator = storage_t::const_iterator;
	using size_type = storage_t::size_type;

	data_value::ptr operator[](std::size_t idx) { return storage[idx]; }
	data_value::ptr at(std::size_t idx) { return storage.at(idx); }
	std::size_t size() const { return storage.size(); }
	iterator begin() { return storage.begin(); }
	iterator end() { return storage.end(); }
	const_iterator begin() const { return storage.begin(); }
	const_iterator end() const { return storage.end(); }
};

class data {
	using storage_t = std::vector<data_list::ptr>;
	std::string _path;
	storage_t _lists;
	config::ptr _config;
	uint16_t _version = 0;

	void parse(std::ifstream& ifs, const std::vector<config::ptr>& confs);
public:
	using value_type = storage_t::value_type;
	using iterator = storage_t::const_iterator;
	using const_iterator = storage_t::const_iterator;
	using size_type = storage_t::size_type;

	explicit data(const char* path);
	void load(config::ptr conf);
	void load(const std::vector<config::ptr>& conf);
	void save(const char* path = nullptr);

	uint16_t version() const { return _version; }

	data_list::ptr get_list(data_type list) {
		return _lists.at(static_cast<int>(list) - 1); // first list has index = 1
	}

	data_list::ptr get_list(const std::string& name) {
		return get_list(_config->get_dt_by_name(name));
	}

	std::size_t size() const { return _lists.size(); }
	iterator begin() { return _lists.begin(); }
	iterator end() { return _lists.end(); }
	const_iterator begin() const { return _lists.begin(); }
	const_iterator end() const { return _lists.end(); }

	using ptr = std::shared_ptr<data>;
};

}