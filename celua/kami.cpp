#include "../lua53/src/lua.hpp"
#include "../Inject/luacall.hpp"
#include "../tool/md5.h"
#include "../tool/JsonObject.h"
#include "connect.h"
#include "globalvalue.h"
#include "luaglobal.h"

using namespace std;

int chongcard_chongcardClick(lua_State* L)
{
	std::string data[6]{ "username", lua_getvalue<std::string>(L, 0, "chongcard.account.Text") ,
		"password", lua_getvalue<std::string>(L, 0, "chongcard.mima.Text") ,
		"recharge_card", lua_getvalue<std::string>(L, 0, "chongcard.kami.Text") };
	auto response = encPost("https://www.baidu.com", data, 3);
	auto json = det::JsonObject(response);
	if (json.getInt("code") == 200)
		MessageBoxA(0, "充值卡密成功，登陆后将能看到剩余使用时间", "卡密充值", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "网络连接失败，请检查你的网络连接。", "卡密充值", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "卡密充值", 0);
	return 0;
}