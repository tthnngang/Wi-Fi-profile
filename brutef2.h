#pragma once
#include <thread>
#include <iostream>
#include <windows.h>
#include <wlanapi.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "wlanapi.lib")

void listWifiInfo();
void writeXml(const std::string profilePath, const std::string& newName, const std::string& newHex, const std::string& newKey);
std::string wstringToHex(const std::wstring& input);
std::string stringToHex(const std::string& input);
int isConnected();