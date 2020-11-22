//
// Created by int3 on 15.02.19.
//

#include "encoding.h"

#include <cstring>
#include <external/iconv.hpp>

template <class Char>
std::basic_string<Char> drop_end_zero(std::basic_string<Char> str) {
    auto pos = str.find(Char(0));
    if (pos != str.npos) {
        str.resize(pos);
    }
    return str;
}

std::string gbk_to_utf8(const std::string& input) {
    static const iconvpp::converter conv{ "UTF-8", "GBK", true };

    std::string output;
    conv.convert(input, output);
    return output;
}

std::string utf8_to_gbk(const std::string& input) {
    static const iconvpp::converter conv{ "GBK", "UTF-8", true };

    std::string output;
    conv.convert(input, output);
    return output;
}

std::string unicode_to_utf8(const std::u16string& input) {	
    static const iconvpp::converter conv{ "UTF-8", "UTF-16LE", true };

    std::string input8(input.length() * 2, '\0');
    std::memcpy(&input8[0], input.data(), input.length() * 2);

    std::string output;
    conv.convert(input8, output);
    return output;
}

std::u16string utf8_to_unicode(const std::string& input) {		
    static const iconvpp::converter conv{ "UTF-16LE", "UTF-8", true };

    std::string output;
    conv.convert(input, output);

    auto p = reinterpret_cast<const char16_t*>(output.data());
    return std::u16string{ p, p + output.size() / 2 };
}

std::u16string gbk_to_unicode(const std::string& input) {
    static const iconvpp::converter conv{ "UTF-16LE", "GBK", true };
    std::string output;
    conv.convert(input, output);

    auto p = reinterpret_cast<const char16_t*>(output.data());
    return std::u16string{ p, p + output.size() / 2 };
}

std::string unicode_to_gbk(const std::u16string& input) {	
    static const iconvpp::converter conv{ "GBK", "UTF-16LE", true };

    std::string input8(input.length() * 2, '\0');
    std::memcpy(&input8[0], input.data(), input.length() * 2);

    std::string output;
    conv.convert(input8, output);
    return output;
}

std::string to_utf8(const char* str, size_t length) {
    return drop_end_zero(gbk_to_utf8(std::string(str, length)));
}

std::string to_utf8(const char16_t* str, size_t length) {
    return drop_end_zero(unicode_to_utf8({ str, str + length }));
}

void from_utf8(const std::string& input, char* out, size_t length) {
    auto str = drop_end_zero(utf8_to_gbk(input));
    if (str.length() >= length) {
        char buf[100];
        sprintf(buf, "[from_utf8: 1-byte] too big string (got: %lu, max: %lu)", str.length(), length);
        throw std::runtime_error(buf);
    }
    std::copy(str.begin(), str.end(), out);
    std::fill(out + str.length(), out + length, '\0');
}

void from_utf8(const std::string& input, char16_t* out, size_t length) {
    auto str = drop_end_zero(utf8_to_unicode(input));
    if (str.length() >= length) {
        char buf[100];
        sprintf(buf, "[from_utf8: 2-byte] too big string (got: %lu, max: %lu)", str.length(), length);
        throw std::runtime_error(buf);
    }
    std::copy(str.begin(), str.end(), out);
    std::fill(out + str.length(), out + length, u'\0');
}


