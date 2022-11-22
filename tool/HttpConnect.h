/*
* 预处理器处添加_WINSOCKAPI_，否则头文件包含顺序必须为 "Winsock2.h" "Windows.h" 
* 
* 
## Usage

easy get: std::string response = http::Get("url");
easy post: std::string response = http::Post("url", "body");

### Example of GET request
```cpp
#include "HTTPRequest.hpp"

try
{
    // you can pass http::InternetProtocol::V6 to Request to make an IPv6 request
    http::Request request{"http://test.com/test"};

    // send a get request
    const auto response = request.send("GET");
    std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
}
catch (const std::exception& e)
{
    std::cerr << "Request failed, error: " << e.what() << '\n';
}
```

### Example of POST request
```cpp
#include "HTTPRequest.hpp"

try
{
    http::Request request{"http://test.com/test"};
    // send a post request
    const auto response = request.send("POST", "foo=1&bar=baz", {
        "Content-Type: application/x-www-form-urlencoded"
    });
    std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
}
catch (const std::exception& e)
{
    std::cerr << "Request failed, error: " << e.what() << '\n';
}
```

### Example of POST request with form data
```cpp
#include "HTTPRequest.hpp"

try
{
    http::Request request{"http://test.com/test"};
    const string body = "foo=1&bar=baz";
    const auto response = request.send("POST", body, {
        "Content-Type: application/x-www-form-urlencoded"
    });
    std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
}
catch (const std::exception& e)
{
    std::cerr << "Request failed, error: " << e.what() << '\n';
}
```

### Example of POST request with a JSON body
```cpp
#include "HTTPRequest.hpp"

try
{
    http::Request request{"http://test.com/test"};
    const std::string body = "{\"foo\": 1, \"bar\": \"baz\"}";
    const auto response = request.send("POST", parameters, {
        "Content-Type: application/json"
    });
    std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
}
catch (const std::exception& e)
{
    std::cerr << "Request failed, error: " << e.what() << '\n';
}
```
*/

#ifndef HTTPCONNECT_HPP
#define HTTPCONNECT_HPP
#pragma comment(lib, "ws2_32.lib")//socket编程必须要链接的库，并且预处理器处添加_WINSOCKAPI_
#include <sstream>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

#if defined(_WIN32) || defined(__CYGWIN__)
#  pragma push_macro("WIN32_LEAN_AND_MEAN")
#  pragma push_macro("NOMINMAX")
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif // WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif // NOMINMAX
#  include <winsock2.h>
#  if _WIN32_WINNT < _WIN32_WINNT_WINXP
extern "C" char* _strdup(const char* strSource);
#    define strdup _strdup
#    include <wspiapi.h>
#  endif // _WIN32_WINNT < _WIN32_WINNT_WINXP
#  include <ws2tcpip.h>
#  pragma pop_macro("WIN32_LEAN_AND_MEAN")
#  pragma pop_macro("NOMINMAX")
#else
#  include <errno.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif // defined(_WIN32) || defined(__CYGWIN__)

namespace http
{
    std::string Get(std::string const& url);

    std::string Post(std::string const& url, std::string const& body);
}

#endif // HTTPCONNECT_HPP