#include <iostream>
#include <windows.h>
using namespace std;

// Need to define a windows version macro? (_WIN32_WINNT didn't help)
typedef LONG LSTATUS;
const static int RRF_RT_REG_BINARY = 0x8;
extern "C" WINAPI LSTATUS RegGetValueA(
  HKEY    hkey,
  LPCSTR  lpSubKey,
  LPCSTR  lpValue,
  DWORD   dwFlags,
  LPDWORD pdwType,
  PVOID   pvData,
  LPDWORD pcbData
);


int main() {
    const char *keyPath = "Software\\Classes\\Local Settings"
        "\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify";
    HKEY hKey;
    LSTATUS status;

    status = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath, 0, KEY_READ, &hKey);
    if(status) {
        cerr << "Failed to read key with error: " << status << endl;
        return 1;
    }

    // TODO: Call with zero length buffer to get the right size
    const static int bufLen = 1024 * 1024;
    unsigned char buffer[bufLen];
    DWORD mBufLen = bufLen;
    status = RegGetValueA(hKey, nullptr, "IconStreams", RRF_RT_REG_BINARY, nullptr, buffer, &mBufLen);

    if(status) {
        cerr << "Failed to read data with error: " << status << endl;
        return 3;
    }

    for(int i =0; i<10; i++) {
        printf("%02x ", (unsigned int)(buffer[i]));
    }
    printf("\n");

    return 0;
}