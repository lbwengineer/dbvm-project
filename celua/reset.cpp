#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "connect.h"
#include "globalvalue.h"
#include "luaglobal.h"

using namespace std;


int Reset_getcrcClick(lua_State* L)
{
	std::string data[4]{ "username", lua_getvalue<std::string>(L, 0, "Reset.email.Text"),"type","seek"};
	auto response = encPost("https://www.baidu.com", data, 2);
	auto json = det::JsonObject(response);
	if (json.getInt("code") == 200)
		MessageBoxA(0, "验证码已经发送到你的邮箱", "发送验证码", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "网络连接失败，请检查你的网络连接。", "发送验证码", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "发送验证码", 0);
	return 0;
}

int Reset_ResetClick(lua_State* L)
{
	std::string data[6]{ "username", lua_getvalue<std::string>(L, 0, "Reset.email.Text"),
		"password", lua_getvalue<std::string>(L, 0, "Reset.Newpassword.Text") ,
		"markcode", lua_getvalue<std::string>(L, 0, "Reset.crc.Text") };
	auto response = encPost("https://www.baidu.com", data, 3);
	auto json = det::JsonObject(response);
	if (json.getInt("code") == 200)
		MessageBoxA(0, "重置密码成功，请使用新密码登录。", "重置密码", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "网络连接失败，请检查你的网络连接。", "重置密码", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "重置密码", 0);
	return 0;
}