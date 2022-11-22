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
		MessageBoxA(0, "��֤���Ѿ����͵��������", "������֤��", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "��������ʧ�ܣ���������������ӡ�", "������֤��", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "������֤��", 0);
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
		MessageBoxA(0, "��������ɹ�����ʹ���������¼��", "��������", 0);
	else if (json.getInt("code") == 0)
		MessageBoxA(0, "��������ʧ�ܣ���������������ӡ�", "��������", 0);
	else
		MessageBoxA(0, json.getJsonObject("data").getString("msg").c_str(), "��������", 0);
	return 0;
}