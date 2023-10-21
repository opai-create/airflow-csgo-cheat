#include "win_http.h"
#include "xorstr.hpp"
#include <libs.h>

std::string WININET::http_request( const std::string& _server, const std::string& _page, const std::string& _params )
{
    char szData[1024];

    // initialize WinInet
    HINTERNET hInternet = WINCALL(InternetOpenA)( CXOR("Balls"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
    if ( hInternet != NULL )
    {
        // open HTTP session
        HINTERNET hConnect = WINCALL(InternetConnectA)( hInternet, _server.c_str( ), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1 );
        if ( hConnect != NULL )
        {
            std::string request = _page +
                ( _params.empty( ) ? "" : ( "?" + _params ) );

            // open request
            HINTERNET hRequest = WINCALL(HttpOpenRequestA)( hConnect, CXOR("GET"), request.c_str( ), NULL, NULL, 0, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_SECURE, 1 );
            if ( hRequest != NULL )
            {
                // send request
                BOOL isSend = WINCALL(HttpSendRequestA)( hRequest, NULL, 0, NULL, 0 );

                if ( isSend )
                {
                    for ( ;; )
                    {
                        // reading data
                        DWORD dwByteRead;
                        BOOL isRead = WINCALL(InternetReadFile)( hRequest, szData, sizeof( szData ) - 1, &dwByteRead );

                        // break cycle if error or end
                        if ( isRead == FALSE || dwByteRead == 0 )
                            break;

                        // saving result
                        szData[dwByteRead] = 0;
                    }
                }

                // close request
                WINCALL( InternetCloseHandle )( hRequest );
            }
            // close session
            WINCALL( InternetCloseHandle )( hConnect );
        }
        // close WinInet
        WINCALL( InternetCloseHandle )( hInternet );
    }

    return std::string( szData );
}
