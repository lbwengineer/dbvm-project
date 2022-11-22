#pragma once
/*
个人写的小工具，只能解析一些简单的json字符串
*/

#include <iostream>
#include <cstring>

/*

*/

std::string unicode2ansi(const std::string& str, int encoding = 0);

namespace det {
	class JsonObject {
	private:
		std::string json;
		static const char* jsonForNext(const char* next);
	public:
		JsonObject(std::string const& json);
		int getInt(const char* key);
		std::string getString(const char* key, int encoding = 0);
		JsonObject getJsonObject(const char* key);
		std::string getJson();
	};
}