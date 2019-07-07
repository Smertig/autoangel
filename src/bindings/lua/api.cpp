//
// Created by int3 on 14.02.19.
//

#include <src/elements/config.h>
#include <src/elements/data.h>

#include <sol.hpp>

#include <iostream>

sol::object get(const elements::field_value& self, sol::this_state L) {
	sol::object result;
	self.get([&](const auto& value) {
		result = sol::make_object(L, value);
	});
	return result;
}

void set(elements::field_value& self, sol::stack_object obj) {
	self.set([&obj](auto& value) {
		using T = std::decay_t<decltype(value)>;
		value = obj.as<T>();
	});
}

sol::object data_value_getter(const elements::data_value& self, sol::stack_object key, sol::this_state L) {
	// we use stack_object for the arguments because we know
	// the values from Lua will remain on Lua's stack,
	// so long we we don't mess with it
	auto string_key = key.as<std::string>();
	decltype(self.value)::const_iterator it = self.value.find(string_key);
	if (it != self.value.end()) {
		return get(it->second, L);
	}
	else {
		return sol::nil;
	}
}

void data_value_setter(elements::data_value& self, sol::stack_object key, sol::stack_object value) {
	// we use stack_object for the arguments because we know
	// the values from Lua will remain on Lua's stack,
	// so long we we don't mess with it
	auto string_key = key.as<std::string>();
	decltype(self.value)::iterator it = self.value.find(string_key);
	if (it != self.value.end()) {
		set(it->second, value);
	}
	else {
		// nothing
	}
}

struct data_value_iterator_state {
	using iterator = decltype(elements::data_value::value)::iterator;

	iterator it;
	iterator last;

	explicit data_value_iterator_state(elements::data_value& data) : it(data.value.begin()), last(data.value.end()) {}
};

std::tuple<sol::object, sol::object> data_value_next(sol::user<data_value_iterator_state&> user_it_state, sol::this_state L) {
	// this gets called
	// to start the first iteration, and every
	// iteration there after

	// the state you passed in my_pairs is argument 1
	// the key value is argument 2, but we do not
	// care about the key value here
	data_value_iterator_state& it_state = user_it_state;
	auto& it = it_state.it;
	if (it == it_state.last) {
		// return nil to signify that
		// there's nothing more to work with.
		return std::make_tuple(
				sol::object(sol::lua_nil),
				sol::object(sol::lua_nil)
		);
	}
	auto itderef = *it;
	// 2 values are returned (pushed onto the stack):
	// the key and the value
	// the state is left alone
	auto r = std::make_tuple(sol::object(L, sol::in_place, it->first), get(it->second, L));
	// the iterator must be moved forward one before we return
	std::advance(it, 1);
	return r;
}

auto data_value_pairs(elements::data_value& mt) {
	// pairs expects 3 returns:
	// the "next" function on how to advance,
	// the "table" itself or some state,
	// and an initial key value (can be nil)

	// prepare our state
	data_value_iterator_state it_state(mt);
	// sol::user is a space/time optimization over regular usertypes,
	// it's incompatible with regular usertypes and stores the type T directly in lua without any pretty setup
	// saves space allocation and a single dereference
	return std::make_tuple(&data_value_next, sol::user<data_value_iterator_state>(std::move(it_state)), sol::lua_nil);
}

void init(sol::state_view lua_state) {
	using namespace elements;

	{
		sol::table elements = lua_state["elements"] = lua_state.create_table_with();

		elements["load_cfg"] = &config::load;
		elements["load_cfgs"] = &config::load_folder;

		elements.new_enum<data_type>("data_type", {});
		elements.new_enum<space_id>("space_id", {
				{ "essence", space_id::essence },
				{ "addon", space_id::addon },
				{ "talk", space_id::talk },
				{ "face", space_id::face },
				{ "recipe", space_id::recipe },
				{ "config", space_id::config },
		});

		auto config_generic_get = [](config* self, auto key, sol::this_state L) -> list_config::ptr {
			try {
				return self->get_list(key);
			}
			catch (std::exception& e) {
				std::cerr << "error: " << e.what() << "\n";
				return nullptr;
			}
		};
		elements.new_usertype<config>("elements::config",
				"version", sol::readonly_property(&config::version),
				//"lists", sol::property(&config::get_lists),
				sol::meta_function::index, sol::overload(
					static_cast<list_config::ptr(*)(config*, data_type, sol::this_state)>(config_generic_get),
					static_cast<list_config::ptr(*)(config*, const std::string&, sol::this_state)>(config_generic_get)
				)
		);

		elements.new_usertype<list_config>("elements::list_config",
				"offset", &list_config::offset,
				"caption", &list_config::caption/*,
				"fields", sol::readonly(&list_config::fields)*/
		);

		auto data_generic_get = [](data* self, auto key, sol::this_state L) -> data_list::ptr {
			try {
				return self->get_list(key);
			}
			catch (std::exception& e) {
				std::cerr << "error: " << e.what() << "\n";
				return nullptr;
			}
		};
		elements.new_usertype<data>("data",
				sol::call_constructor, sol::constructor_list<data(const char*)>(),
				"version", sol::readonly_property(&data::version),
				"load", sol::overload(
						static_cast<void(data::*)(config::ptr)>(&data::load),
						static_cast<void(data::*)(const std::vector<config::ptr>&)>(&data::load)
				),
				sol::meta_function::index, sol::overload(
						static_cast<data_list::ptr(*)(data*, data_type, sol::this_state)>(data_generic_get),
						static_cast<data_list::ptr(*)(data*, const std::string&, sol::this_state)>(data_generic_get)
				),
				"save", &data::save
		);

		elements.new_usertype<data_list>("data_list",
				"type", sol::readonly(&data_list::type),
				"space", sol::readonly(&data_list::space)
		);

		elements.new_usertype<data_value>("data_value",
				sol::meta_function::index, &data_value_getter,
				sol::meta_function::new_index, &data_value_setter,
				sol::meta_function::pairs, &data_value_pairs,
				"clone", [](const data_value& rhs) -> data_value { return rhs.clone(); }
		);
	}

	{
		sol::table pck = lua_state["pck"] = lua_state.create_table_with();
	}
}

extern "C" {

int luaopen_autoangel(lua_State* L){
	init(sol::state_view{ L });
	return 1;
}

}