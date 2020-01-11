//
// Created by int3 on 1/11/20.
//

#include <src/pck/package.h>
#include <src/util/encoding.h>
#include <fmt/format.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		fmt::print(stderr, "usage: {} <path-to-pck>", argv[0]);
		return 1;
	}

	try {
		pck::package test{ argv[1] };

		test.load();

		const auto range = test.find_prefix(u"");

		fmt::print("{} file(s):\n", range.second - range.first);
		for (auto it = range.first; it != range.second; ++it) {
			fmt::print("{} -> {} bytes\n", unicode_to_utf8(it->filename), it->size);
		}

		return 0;
	}
	catch (std::exception& e) {
		fmt::print(stderr, "{} fails: {}\n", argv[0], e.what());
		return 1;
	}
}
