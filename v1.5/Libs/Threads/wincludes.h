#ifdef _WIN32
#ifndef WINCLUDES_H
#define WINCLUDES_H

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <intrin.h>

#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Mswsock.lib" )
#pragma comment( lib, "AdvApi32.lib" )
#endif
#endif
