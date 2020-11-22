#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <src/elements/data.h>
#include <src/elements/config.h>
#include <src/pck/package.h>

#include <regex>

namespace py = pybind11;

// C++ to python

namespace detail {

template <std::size_t N>
struct priority_tag : priority_tag<N - 1> {};

template <>
struct priority_tag<0> {};

// utf8 string
std::u16string to_python(const std::string& input, priority_tag<2>) {
    return utf8_to_unicode(input);
}

// everything else
template <class T>
T to_python(T&& value, priority_tag<1>) {
    return std::forward<T>(value);
}

}

template <class T>
auto to_python(T&& value) {
    return detail::to_python(std::forward<T>(value), detail::priority_tag<10>{});
}

#if 0
template <class F>
void get(const elements::field_value& self, F&& f) {
    self.get(std::forward<F>(f));
}

template <class F>
void set(elements::field_value& self, F&& f) {
    self.set(std::forward<F>(f));
}
#endif

py::object data_value_getter(const elements::data_value& data, const char* field) {
    auto it = data.value.find(field);
    if (it != data.value.end()) {
        py::object obj;
        it->second.get([&obj](const auto& value) { obj = py::cast(to_python(value)); });
        return obj;
    }
    else {
        return py::none{};
    }
}

void data_value_setter(elements::data_value& data, const char* field, py::object obj) {
    auto it = data.value.find(field);
    if (it != data.value.end()) {
        it->second.set([&obj](auto& value) { value = py::cast<std::decay_t<decltype(value)>>(obj); });
    }
    else {
        // nothing
    }
}

py::iterator data_value_iter(elements::data_value& data) {
    struct iter_t : elements::data_value::iterator {
        using iterator = elements::data_value::iterator;

        iter_t(iterator base) : iterator(base) {}

        std::pair<std::string, py::object> operator*() const {
            auto& p = iterator::operator*();
            py::object obj;
            p.second.get([&obj](const auto& value) { obj = py::cast(to_python(value)); });
            return std::make_pair(p.first, std::move(obj));
        }
    };
    return py::make_iterator(iter_t{ data.begin() }, iter_t{ data.end() });
}

PYBIND11_MODULE(autoangel, m) {
    m.doc() = "AutoAngel module for elements.data editing";

    {
        auto elements = m.def_submodule("elements", "elements.data submodule");
        using namespace elements;

        elements.def("load_cfg", &config::load, "Loads config from file");
        elements.def("load_cfgs", &config::load_folder, "Loads configs from folder");

        py::enum_<data_type>(elements, "data_type");
        py::enum_<space_id>(elements, "space_id")
                .value("essence", space_id::essence)
                .value("addon", space_id::addon)
                .value("talk", space_id::talk)
                .value("face", space_id::face)
                .value("recipe", space_id::recipe)
                .value("config", space_id::config)
                ;

        py::class_<config, config::ptr>(elements, "config")
                .def("__getitem__", static_cast<list_config::ptr(config::*)(data_type)>(&config::get_list))
                .def("__getitem__", static_cast<list_config::ptr(config::*)(const std::string&)>(&config::get_list))
                .def("__iter__", [](const config& self) { return py::make_iterator(self); }, py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
                .def_property_readonly("version", &config::version)
                ;

        py::class_<list_config, list_config::ptr>(elements, "list_config")
                //.def("__getitem__", &list_config::at)
                .def_readonly("offset", &list_config::offset)
                .def_readonly("caption", &list_config::caption)
                .def_readonly("fields", &list_config::fields)
                ;

        py::class_<field_meta>(elements, "field_meta")
                .def_readonly("name", &field_meta::first)
                .def_readonly("type", &field_meta::second)
                //.def("__getitem__", &field_meta::at)
                ;

        py::class_<meta_type>(elements, "meta_type")
                ;

        py::class_<data, data::ptr>(elements, "data")
                .def(py::init<const char*>())
                .def_property_readonly("version", &data::version)
                .def("load", static_cast<void(data::*)(config::ptr)>(&data::load), "Loads elements.data")
                .def("load", static_cast<void(data::*)(const std::vector<config::ptr>&)>(&data::load), "Loads elements.data")
                .def("__getitem__", static_cast<data_list::ptr(data::*)(data_type)>(&data::get_list))
                .def("__getitem__", static_cast<data_list::ptr(data::*)(const std::string&)>(&data::get_list))
                .def("__iter__", [](const data& self) { return py::make_iterator(self); }, py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
                .def("save", &data::save, py::arg("path") = nullptr, py::arg("config") = nullptr)
                ;

        py::class_<data_list, data_list::ptr>(elements, "data_list")
                .def("__getitem__", [](data_list& self, std::size_t index) { return self.at(index); })
                .def_readonly("type", &data_list::type)
                .def_readonly("space", &data_list::space)
                .def_readonly("caption", &data_list::caption)
                .def("append", [](data_list& self, data_value::ptr ptr) { self.storage.emplace_back(std::move(ptr)); })
                ;

        py::class_<data_value, data_value::ptr>(elements, "data_value")
                .def("__getattr__", &data_value_getter)
                .def("__setattr__", &data_value_setter)
                .def("__iter__", &data_value_iter, py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
                .def("clone", [](const data_value& rhs) -> data_value { return rhs.clone(); })
                ;
    }

    {
        auto pck = m.def_submodule("pck", "PCK packages submodule");
        using namespace pck;

        py::class_<package>(pck, "package")
                .def(py::init<std::string>())
                .def("load", &package::load)
                .def("read", &package::read)
                .def("find_prefix", [](package& p, std::u16string prefix) {
                    auto rng = p.find_prefix(std::move(prefix));
                    using base_t = std::decay_t<decltype(rng.first)>;
                    struct iter_t : base_t {
                        iter_t(base_t b) : base_t(b) {}

                        const std::u16string& operator*() const {
                            return base_t::operator->()->filename;
                        }
                    };
                    return py::make_iterator(iter_t{ rng.first }, iter_t{ rng.second });
                })
                ;
    }
}
