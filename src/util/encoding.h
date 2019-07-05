//
// Created by int3 on 15.02.19.
//

#pragma once

#include <string>

std::string gbk_to_utf8(const std::string& input);
std::string utf8_to_gbk(const std::string& input);
std::string unicode_to_utf8(const std::u16string& input);
std::u16string utf8_to_unicode(const std::string& input);
std::u16string gbk_to_unicode(const std::string& input);
std::string unicode_to_gbk(const std::u16string& input);

std::string to_utf8(const char* str, size_t length);
std::string to_utf8(const char16_t* str, size_t length);
void from_utf8(const std::string& input, char* out, size_t length);
void from_utf8(const std::string& input, char16_t* out, size_t length);