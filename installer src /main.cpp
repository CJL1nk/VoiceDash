#include <iostream>
#include <Windows.h>
#include <filesystem>

const char* file1 = "VoiceDash.dll";
const char* file2 = "portaudio_x86.dll";

// this entire function came from chatgpt LMFAO
bool GetSteamDirectory(std::wstring& steamDir) {

    HKEY hKey;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", 0, KEY_READ | KEY_WOW64_32KEY, &hKey) != ERROR_SUCCESS) {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS) {
            std::wcout << L"Failed to open Steam registry key." << std::endl;
            return false;
        }
    }

    WCHAR steamPath[MAX_PATH];
    DWORD bufferSize = sizeof(steamPath);

    if (RegGetValueW(hKey, nullptr, L"InstallPath", RRF_RT_REG_SZ, nullptr, steamPath, &bufferSize) != ERROR_SUCCESS) {
        std::wcout << L"Failed to retrieve Steam installation directory." << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    steamDir = steamPath;
    return true;
}

int main() {

    std::wstring steamDir;

    if (GetSteamDirectory(steamDir)) {

        std::wstring gdDir = steamDir + L"\\steamapps\\common\\Geometry Dash";
        std::wcout << L"Geometry Dash directory: " << gdDir << std::endl;

        std::wstring extensionsDir = gdDir + L"\\extensions";

        std::filesystem::path sourcePath1 = std::filesystem::current_path() / file1;
        std::filesystem::path sourcePath2 = std::filesystem::current_path() / file2;
        std::filesystem::path destinationPath1 = std::filesystem::path(extensionsDir) / file1;
        std::filesystem::path destinationPath2 = std::filesystem::path(gdDir) / file2;

        std::error_code errorCode;
        std::filesystem::rename(sourcePath1, destinationPath1, errorCode);
        std::filesystem::rename(sourcePath2, destinationPath2, errorCode);

        if (errorCode) {
            std::cerr << "Error moving files: " << errorCode.message() << std::endl;
            return 1;
        }

        std::cout << "Files moved successfully." << std::endl;
    }
    else {
        std::cout << "Failed to get Geometry Dash directory. 32 bit user lol" << std::endl;
    }

    system("pause");

    return 0;
}
