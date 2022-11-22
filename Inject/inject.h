#pragma once
#include <Windows.h> 
#include <string>
#include <vector>
#include <iostream>
#include <process.h>
#include <processsnapshot.h>
#include <tlhelp32.h>


struct REGION {
	PBYTE BaseAddress;
	PBYTE AllocationBase;
	DWORD Protect;
	DWORD AllocationProtect;
	DWORD State;
	DWORD Type;
	size_t RegionSize;
};

class HOOK
{
private:
	DWORD target_pid;
	BOOL myself;
	HANDLE target_handle;
public:
	HOOK(DWORD pid);
	~HOOK();
	/*
	Զ�̷����д�ڴ�
	size	������ڴ��С
	*/
	PVOID malloc(size_t size = 0x1000);
	/*
	��ȡ�ڴ�ķ�����Ϣ
	*/
	std::vector< REGION> getRegions();
	/*
	Զ�̷����дִ���ڴ�
	baseAddr	Ҫ����ĵ�ַӦ�þ����ܿ����˵�ַ
	size		������ڴ��С
	*/
	PVOID mmapexec(PVOID baseAddr = 0, size_t size = 0x1000);
	/*
	�ͷŷ�����ڴ�
	freeAddr	Ҫ�ͷŵ��ڴ�ĵ�ַ
	*/
	BOOL free(PVOID freeAddr);
	/*
	Զ���߳�ִ��Զ�̺���
	funcAddr	Զ�̺�����ַ
	params		Զ�̺����Ĳ�����ע��������ֻ��һ��ֻ��Ҫǿ��ת����PVOID���ɣ�
	��������ж����Ҫʹ�ýṹ��ѽṹ��д��Զ���ڴ�Ȼ���Զ�̵�ַ����ȥ����
	*/
	DWORD call(PVOID funcAddr, PVOID params);
	/*
	��ָ���Ľ���ע����Ӧ��ģ��
    dllPath		��ע���dll������·��
	*/
	PVOID dllinject(LPCWSTR dllPath);
	/*
	�ӽ������Ƴ�ģ�飬ע�뼸�ξ�Ҫ�Ƴ�����
	dllPath		��Ҫ�Ƴ���dll��ģ�����ƻ�����·��
	*/
	BOOL dlleject(LPCWSTR dllNamePath);
	/*
	��ȡԶ�̽��̵��ڴ��ַ����ֵ
	address		Զ�̽��̵��ڴ��ַ
	readdata	���ؽ��̵��ڴ��ַ
	readBytes	Ҫ��ȡ�����ݵ��ֽ���
	*/
	BOOL readdata(PVOID address, PVOID readdata, size_t readBytes);
	/*
	д��Զ�̽��̵��ڴ��ַ����ֵ
	address		Զ�̽��̵��ڴ��ַ
	writedata	���ؽ��̵��ڴ��ַ
	writeBytes	Ҫд������ݵ��ֽ���
	*/
	BOOL writedata(PVOID address, LPCVOID writedata, size_t writeBytes);
	/*
	ֻ�ܽ���int, llong, jmp��call����λ��ʮ������
	*/
	size_t writecode(PVOID address, LPCSTR code, ...);
	/*
	�Ѻ������Ƶ��µĵ�ַ��������ԭʼ����
	*/
	PVOID copyfunc(PVOID from_address, PVOID to_address, int copy_lens = 5, PVOID ret_address = 0);
	/*
	��һ������ʹ��inline hook
	new_address		��ԭʼ����hook��ִ�еĵ�ַ
	save_address	����ԭʼ�����ĵ�ַ����������Ϊ0��ʾ����Ҫ����ԭʼ����
	hook_address	��Ҫhook��ԭʼ����
	*/
	BOOL inlifhook(PVOID new_address, PVOID save_address, PVOID hook_address);
	/*
	��ȡһ��ģ���import table address
	module_handle	Ҫ��ȡ��ģ�����ڵĵ�ַ
	func_module		Ҫ��ȡ�ĺ������ڵ�ģ�����ƣ����ִ�Сд������0��ʾ�κ�ģ�鶼����
	func_name		Ҫ��ȡ�ĺ���������
	*/
	PBYTE getiat(PVOID module_handle, LPCWSTR func_module, LPCSTR func_name);
	/*
	hookָ��ģ���import table address
	module_handle	Ҫhook��ģ�����ڵĵ�ַ
	func_module		Ҫhook�ĺ������ڵ�ģ�����ƣ����ִ�Сд������0��ʾ�κ�ģ�鶼����
	func_name		Ҫhook�ĺ���������
	new_address		��func_name���ĵ����ַ�ĳ�new_address��ʹ��module_handleģ���func_name���ñ��new_address����
	*/
	BOOL iathook(PVOID module_handle, LPCWSTR func_module, LPCSTR func_name, PVOID new_address);
	/*
	��ȡָ��dll�ļ���symbol��Ϣ
	module		dll���׵�ַ��Ҳ���Ǿ��
	func_name	dll��ָ��symbol������
	*/
	PVOID dllsym(PVOID module_handle, LPCSTR func_name);
};

class Eile
{
private:
	FILE* fw;
public:
	Eile(const char* path);
	~Eile();
	int write(const char* format, ...);
};

DWORD getpid(LPCWSTR name);
DWORD getpid(LPCSTR name);
PBYTE get_module_addr(DWORD target_pid, LPCWSTR dllNamePath);
std::vector< MODULEENTRY32> get_module_aaddr(DWORD target_pid);
BOOL getSpecDll(DWORD target_pid, LPCVOID funcAddr, LPWSTR dllName, size_t cDllSize);
PBYTE get_remote_addr(DWORD target_pid, LPCVOID localAddr);
PBYTE createSharedMemory(LPCSTR name, size_t size);
PBYTE createSharedMemory(LPCWSTR name, size_t size);
PBYTE openSharedMemory(LPCSTR name, size_t size);
PBYTE openSharedMemory(LPCWSTR name, size_t size);
/*
���ұ��ؽ��̵��ڴ� [start, end)�������ҵ��ĵ�ַ���׵�ַ��������ҵ������ͷ���NULL
start		��ʼ��ַ������
end			������ַ��������
code		�ֽ����ַ������� 02 43 78 F2 e3 Ba ? ???? 09������ ?2 / 9 / F00 ���������ǲ����Ե�
*/
PVOID searchLocalBytes(PVOID start, size_t range_size, LPCSTR code);