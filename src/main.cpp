#include <iostream>
#include <map>
#include <cassert>
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

/* Source: https://tmintner.wordpress.com/2011/07/08/
    windows-7-notification-area-automation-falling-back-down-the-binary-registry-rabbit-hole/*/
void rot13(wchar_t *str);

struct IconStreamBlockHeader {
    union {
        char unk[20];
        struct {    // Reverse engineered
            DWORD unk0;  // size?
            DWORD unk1;
            WORD unk2;
            WORD unk3;
            DWORD streamCount;
            DWORD unk4;  // size?
        };
    };
};

class IconStream {
public:
    wchar_t path[528 / sizeof(wchar_t)];
    DWORD visibility;
    wchar_t lastTip[512 / sizeof(wchar_t)];
    char unk0[592];  // Possible NID variant?
    DWORD unk1;  //App ID?

    void getPath(std::wstring &out) {
        wchar_t *buffer = new wchar_t[wcslen(path) + sizeof(wchar_t)];
        wcscpy(buffer, path);
        rot13(buffer);
        out.assign(buffer);
        delete[] buffer;
    }
};

typedef std::multimap<std::wstring, IconStream*> StreamSet;
typedef StreamSet::value_type KeyedStream;

void rot13(wchar_t *str) {
    while(*str) {
        if(*str >= 'A' && *str <= 'Z') {
            *str -= 'A';
            *str += 13;
            *str %= 26;
            *str += 'A';
        }
        else if(*str >= 'a' && *str <= 'z') {
            *str -= 'a';
            *str += 13;
            *str %= 26;
            *str += 'a';
        }
        str++;
    }
}

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

    DWORD bufLen;
    status = RegGetValueA(hKey, nullptr, "IconStreams", RRF_RT_REG_BINARY, nullptr, nullptr, &bufLen);
    if(status) {
        cerr << "Failed to read size with error: " << status << endl;
        return 2;
    }

    assert((bufLen - sizeof(IconStreamBlockHeader)) % sizeof(IconStream) == 0);

    unsigned char *buffer = new unsigned char[bufLen];
    status = RegGetValueA(hKey, nullptr, "IconStreams", RRF_RT_REG_BINARY, nullptr, buffer, &bufLen);

    if(status) {
        cerr << "Failed to read data with error: " << status << endl;
        return 3;
    }

    IconStreamBlockHeader *header = (IconStreamBlockHeader *)buffer;
    buffer += sizeof(IconStreamBlockHeader);
    bufLen -= sizeof(IconStreamBlockHeader);
    int numStreams = bufLen / sizeof(IconStream);

    printf("Header: ");
    for(int i = 0; i < sizeof(IconStreamBlockHeader); i++) {
        printf("%02x ",(int)(header->unk[i]));
    }
    printf("\n");

    StreamSet streams;
    for(int i = 0; i < numStreams; i++) {
        IconStream *stream = (IconStream*)(buffer);
        stream += i;
        wstring path;
        stream->getPath(path);
        streams.insert(KeyedStream(path, stream));
    }

    for(auto i = streams.begin(); i!=streams.end(); i++){
        printf("%S\n", i->first.c_str());
    }
    printf("\n");

    return 0;
}