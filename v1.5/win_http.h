#pragma once
#include <string>
#include <windows.h>
#include <WinInet.h>
#pragma comment (lib, "Wininet.lib")

namespace WININET
{
    std::string http_request( const std::string& _server,
        const std::string& _page,
        const std::string& _params );
}