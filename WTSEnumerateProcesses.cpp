// API CALL -> WTSEnumerateProcesses
// API CALL-> CreateFile
//------------------------------------------------------------------------------------------INCLUDES
#include <iostream>
#include <Windows.h>
#include <Wtsapi32.h>
#include <Sddl.h>
#include <vector>
#include <string>
#pragma comment(lib, "Wtsapi32.lib")


//------------------------------------------------------------------------------------------WRITE TO FILE FUNCTION
//------------------------------------------------------------------------------------------API CALL -> CreateFile
bool WriteToFile(const std::wstring& filePath, const std::wstring& data)
{
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        if (WriteFile(hFile, data.c_str(), static_cast<DWORD>(data.length() * sizeof(wchar_t)), &bytesWritten, nullptr)) {
            CloseHandle(hFile);
            return true;
        }
        else {
            CloseHandle(hFile);
            return false;
        }
    }
    else {
        return false;
    }
}

//------------------------------------------------------------------------------------------MAIN FUNCTION
int main() {
    PWTS_PROCESS_INFO pProcessInfo = nullptr;
    DWORD count = 0;

    if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pProcessInfo, &count)) { // API CALL -> WTSEnumerateProcesses
        std::wstring output;

        for (DWORD i = 0; i < count; i++) {
            output += L"Process ID (PID): " + std::to_wstring(pProcessInfo[i].ProcessId) + L"\n";
            output += L"Session ID: " + std::to_wstring(pProcessInfo[i].SessionId) + L"\n";

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pProcessInfo[i].ProcessId);
            if (hProcess) {
                WCHAR processName[MAX_PATH];
                DWORD processNameLen = MAX_PATH;

                if (QueryFullProcessImageNameW(hProcess, 0, processName, &processNameLen)) {
                    output += L"Process Name: " + std::wstring(processName) + L"\n";
                }
                else {
                    output += L"Failed to retrieve process name for PID " + std::to_wstring(pProcessInfo[i].ProcessId) + L"\n";
                }

                CloseHandle(hProcess);
            }
            else {
                output += L"Failed to open process for PID " + std::to_wstring(pProcessInfo[i].ProcessId) + L"\n";
            }

            output += L"User SID: ";

            // Convert SID to readable string format
            LPWSTR pSidString = nullptr;

            if (ConvertSidToStringSidW(pProcessInfo[i].pUserSid, &pSidString)) {
                output += std::wstring(pSidString);
                LocalFree(pSidString);
            }
            else {
                output += L"Failed to Retrieve SID for PID " + std::to_wstring(pProcessInfo[i].ProcessId);
            }

            output += L"\n";

            output += L"------------------------------------------------------------------------------------\n\n";
        }

        std::wstring filePath = L"C:\\temp\\output.txt";
        WriteToFile(filePath, output);
    }
    else {
        std::cerr << "WTSEnumerateProcesses Failed. ERROR: " << GetLastError() << std::endl;
        return 1;
    }

    return 0;
}
