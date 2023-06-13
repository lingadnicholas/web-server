#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>


namespace beast = boost::beast;
namespace http = beast::http;

class HttpParser {
    public:
        HttpParser();
        void getFields(std::string request_string, http::request<http::string_body>& req);
        std::string getResponse(http::response<http::string_body> response);
};

#endif
