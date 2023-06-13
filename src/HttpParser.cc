#include <algorithm>
#include <bits/stdc++.h> // needed to convert char array to string
#include <cstring> // needed for strcpy
#include <boost/lexical_cast.hpp>
#include "HttpParser.h"
#include <log.h>
#include <string.h> // needed for memset
#include <vector>

using boost::asio::ip::tcp;

HttpParser::HttpParser() { 
};

void HttpParser :: getFields(std::string request_string, http::request<http::string_body>& req) {
    std::string header_temp = "";
    std::string method;
    std::string requestURI;
    std::vector<std::string> headers;
    std::string httpVersion;
    std::string body;
    bool isBadSyntax = false;
    int stage = 0; // looking for method
    // 1 --> found method, looking for requestURI
    // 2 --> found requestURI, looking for httpVersion
    // 3 --> found httpVersion, looking for header
    // 4 --> found header, looking for message body

    for(int i = 0; i < request_string.length(); i++) {
        switch (stage) {
            case 0:
                // keep reading characters to compose the METHOD field of the http request, until we see a space
                if (request_string[i] != ' ') {
                    method += request_string[i];
                }
                else {
                    if (method == "GET" || method == "HEAD" ||
                        method == "POST" || method == "PUT" ||
                        method == "DELETE" || method == "CONNECT" ||
                        method == "OPTIONS" || method == "TRACE") {
                        if (method == "GET") {
                            req.method(http::verb::get);
                        }
                        else if (method == "HEAD") {
                            req.method(http::verb::head);
                        }
                        else if (method == "POST") {
                            req.method(http::verb::post);
                        }
                        else if (method == "PUT") {
                            req.method(http::verb::put);
                        }
                        else if (method == "DELETE") {
                            req.method(http::verb::delete_);
                        }
                        else if (method == "CONNECT") {
                            req.method(http::verb::connect);
                        }
                        else if (method == "OPTIONS") {
                            req.method(http::verb::options);
                        }
                        else { //  if (method == "TRACE") {
                            req.method(http::verb::trace);
                        }
                        stage++;
                    }
                    else {
                        isBadSyntax = true;
                        LOG_INFO << "HttpParser :: getFields() : invalid request method received" << std::endl;
                    }
                }
                break;
            case 1:
                // keep reading characters to compose the request URI field of the http request, until we see a space
                if (request_string[i] != ' ') {
                    requestURI += request_string[i];
                }
                else {
                    stage++;
                }
                break;
            case 2:
                // keep reading characters to compose the http version field of the http request, until we see a CRLF (\r\n)
                if (request_string[i] != '\r') {
                    httpVersion += request_string[i];
                }
                else {
                    if (i + 1 < request_string.length() && request_string[i + 1] == '\n') {
                        stage++;
                        i++; // skip next character because we already read the \n that comes after the current char (\r) to check for CRLF
                    }
                    else {
                        isBadSyntax = true;
                        LOG_INFO << "session :: getFields(): invalid request received" << std::endl;
                    }
                }
                break;
            case 3:
                // keep reading characters to compose a header line until we see a CRLF, at which point we start reading the next header
                if (request_string[i] != '\r') {
                    header_temp += request_string[i];
                }
                else {
                    if (i + 1 < request_string.length() && request_string[i + 1] == '\n') {
                        if (header_temp == "") { // the request had zero header lines
                            stage++;
                            i++;
                            break;
                        }
                        headers.push_back(header_temp);
                        header_temp = "";
                        // if the header is followed by two CRLFs, that means we are done reading all headers and can move to the next stage
                        if (i + 3 < request_string.length() && request_string[i + 2] == '\r' &&
                            request_string[i + 3] == '\n') {
                            stage++;
                            i += 3; // skip next 3 characters because we already read the \n after the current \r, and the \r\n after that (CRLF CRLF)
                        }
                        else {
                            i++; // skip next char becaues we already read the \n that comes after the current char (\r)
                        }
                    }
                    else {
                        isBadSyntax = true;
                        LOG_INFO << "session :: getFields(): invalid request received" << std::endl;
                    }
                }
                break;
            case 4:
                body += request_string[i];
                break;
        }
        if (isBadSyntax) {
            LOG_INFO << "bad syntax\n";
            break;
        }
    }
    if (stage != 4) {
        isBadSyntax = true;
        LOG_INFO << "session :: getFields(): incomplete/invalid request received" << std::endl;
    }
    req.target(requestURI);

    for (const auto& header : headers) {
        std::size_t pos = header.find(':');
        if (pos != std::string::npos) {
            std::string name = header.substr(0, pos);
            std::string value = header.substr(pos + 2); // Skip the ':' and space after it
            req.set(name, value);
        }
    }

    req.body() = body;
}

std::string HttpParser :: getResponse(http::response<http::string_body> response) {
    std::string res;
    std::string const strHeaders = boost::lexical_cast<std::string>(response.base());
    res += strHeaders;
    res += response.body();
    return res;
}
