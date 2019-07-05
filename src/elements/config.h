//
// Created by int3 on 14.02.19.
//

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <istream>

#include <src/util/encoding.h>

#include <mpark/variant.hpp>

namespace elements {

enum class space_id : int {
	essence = 0,
	addon,
	talk,
	face,
	recipe,
	config
};

enum class data_type : int {};

template <class T>
struct fundamental_meta_type {
	std::size_t size(std::istream&) const {
		return sizeof(T);
	}

	template <class F> // signature void(const T&)
	void get(F&& f, const void* value) {
		std::forward<F>(f)(*reinterpret_cast<const T*>(value));
	}

	template <class F> // signature void(T&)
	void set(F&& f, void* value) {
		std::forward<F>(f)(*reinterpret_cast<T*>(value));
	}
};

template <class Char>
struct string_meta_type {
	size_t length;

	explicit string_meta_type(std::size_t size) : length(size / sizeof(Char)) {}

	std::size_t size(std::istream&) const {
		return length * sizeof(Char);
	}

	template <class F> // signature void(const T&)
	void get(F&& f, const void* value) {
		std::forward<F>(f)(to_utf8(reinterpret_cast<const Char*>(value), length));
	}

	template <class F> // signature void(T&)
	void set(F&& f, void* value) {
		std::string temp;
		std::forward<F>(f)(temp);
		from_utf8(temp, reinterpret_cast<Char*>(value), length);
	}
};

struct raw_meta_type {
	std::size_t size(std::istream& is) const {
		auto skip = [&](size_t count) {
			is.seekg(count, std::istream::cur);
		};

		auto read_some = [&](auto& out) {
			is.read((char*)&out, sizeof(out));
		};

		auto old_pos = is.tellg();
		skip(4);
		skip(0x80);
		uint32_t num_window{};
		read_some(num_window);
		for (uint32_t j = 0; j < num_window; j++) {
			skip(4);
			skip(4);

			uint32_t talk_text_len{};
			read_some(talk_text_len);
			skip(talk_text_len * 2);

			uint32_t num_option{};
			read_some(num_option);
			skip(num_option * 0x88);
		}

		auto size = is.tellg() - old_pos;
		is.seekg(old_pos, std::istream::beg);
		return size;
	}

	template <class F> // signature void(const T&)
	void get(F&& f, const void* value) {
		throw std::runtime_error("not implemented");
	}

	template <class F> // signature void(T&)
	void set(F&& f, void* value) {
		throw std::runtime_error("not implemented");
	}
};

class meta_type {
	using storage_t = mpark::variant<
	        fundamental_meta_type<int32_t>,
	        fundamental_meta_type<float>,
	        string_meta_type<char>,
	        string_meta_type<char16_t>,
	        raw_meta_type
	>;

	storage_t _storage;

	template <class T>
	meta_type(T&& t) : _storage(std::forward<T>(t)) {}
public:
	static meta_type parse(const std::string& str);

	std::size_t size(std::istream& is) const {
		return mpark::visit([&is](const auto& meta) -> std::size_t {
			return meta.size(is);
		}, _storage);
	}

	template <class F> // signature void(const T&)
	void get(F&& f, const void* value) {
		mpark::visit([f = std::forward<F>(f), value](auto&& meta) mutable {
			std::forward<decltype(meta)>(meta).get(std::forward<F>(f), value);
		}, _storage);
	}

	template <class F> // signature void(T&)
	void set(F&& f, void* value) {
		mpark::visit([f = std::forward<F>(f), value](auto&& meta) mutable {
			std::forward<decltype(meta)>(meta).set(std::forward<F>(f), value);
		}, _storage);
	}
};

using field_meta = std::pair<std::string, meta_type>;

struct list_config {
	std::size_t offset;
	std::string caption;
	data_type dt;
	space_id space;
	std::vector<field_meta> fields;

	static std::shared_ptr<list_config> parse(std::istream& is);

	using ptr = std::shared_ptr<list_config>;
};

class config {
	std::string _file_path;
	uint16_t _version;
	std::vector<std::shared_ptr<list_config>> _lists;

public:
	using value_type = decltype(_lists)::value_type;
	using iterator = decltype(_lists)::const_iterator;
	using size_type = decltype(_lists)::size_type;

	using ptr = std::shared_ptr<config>;

	static config::ptr load(std::string path, uint16_t version);
	static std::vector<config::ptr> load_folder(std::string folder);

	std::shared_ptr<list_config> operator[](std::size_t list) {
		return _lists[list];
	}

	std::shared_ptr<list_config> at(std::size_t list) {
		return _lists.at(list);
	}

	const auto& get_lists() const {
		return _lists;
	}

	uint16_t version() const {
		return _version;
	}

	std::size_t size() const {
		return _lists.size();
	}

	iterator begin() const { return _lists.begin(); }
	iterator end() const { return _lists.end(); }
};

}
