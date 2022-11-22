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
		MessageBoxA(0, "��ֵ���ܳɹ�����½���ܿ���ʣ��ʹ��ʱ��", "���ܳ�ֵ", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "��������ʧ�ܣ���������������ӡ�", "���ܳ�ֵ", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "���ܳ�ֵ", 0);
	return 0;
}