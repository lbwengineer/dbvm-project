#pragma once
#include <iostream>

std::string encode(const std::string& text);
std::string decode(const std::string& text);
std::string encPost(const std::string& url, const std::string text[], size_t size);
int time_enc(int _t = 0);
int time_dec(int enc);