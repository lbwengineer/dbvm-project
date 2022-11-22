#include "JsonObject.h"
#include <Windows.h>

std::string unicode2ansi(const std::string& str, int encoding)
{
	auto len = str.size();
	wchar_t* unicode = new wchar_t[len + 1]{};
	size_t u = 0;
	for (size_t i = 0; i < len; i++) {
		if (str[i] == '\\' && str[i + 1] == 'u') {
			auto uni = str.substr(i + 2, 4);
			if (uni.size() != 4)
				break;
			unicode[u++] = strtol(uni.c_str(), 0, 16);
			i += 5;
		}
		else if (str[i] == '\\' && str[i + 1] == '\\') { }
		else if (str[i] == '\\' && str[i + 1] == '/')
			i++, unicode[u++] = '/';
		else if (str[i] == '\\' && str[i + 1] == 'n')
			i++, unicode[u++] = '\n';
		else
			unicode[u++] = str[i];
	}
	char* ansi = new char[len + 1]{};
	WideCharToMultiByte(encoding, 0, unicode, len + 1, ansi, len + 1, 0, 0);
	std::string rets(ansi);
	delete[] unicode, ansi;
	return rets;
}


namespace det {
	const char* JsonObject::jsonForNext(const char* next) {
		auto next_n = strstr(next, "}");
		auto before = strstr(next, "{");
		if (before != 0 && before < next_n)
			next_n = jsonForNext(next_n + 1);
		return next_n;
	}
	JsonObject::JsonObject(std::string const& json) {
		JsonObject::json = json;
	}
	int JsonObject::getInt(const char* key) {
		std::string kes = std::string("\"") + key + std::string("\":");
		auto index = strstr(json.c_str(), kes.c_str());
		if (index == nullptr)
			return 0;
		int len = strlen(key) + 3;
		return strtol(index + len, 0, 10);
	}
	std::string JsonObject::getString(const char* key, int encoding) {
		std::string kes = std::string("\"") + key + std::string("\":\"");
		auto index = strstr(json.c_str(), kes.c_str());
		if (index == nullptr)
			return "";
		int len = strlen(key) + 4;
		return unicode2ansi(std::string(json, index + len - json.c_str(), strstr(index + len, "\"") - index - len), encoding);
	}
	JsonObject JsonObject::getJsonObject(const char* key) {
		std::string kes = std::string("\"") + key + std::string("\":{");
		auto index = strstr(json.c_str(), kes.c_str());
		if (index == nullptr)
			return JsonObject("");
		int len = strlen(key) + 3;
		return std::string(json, index + len - json.c_str(), jsonForNext(index + len + 1) + 1 - index - len);
	}
	std::string JsonObject::getJson() {
		return json;
	}
}