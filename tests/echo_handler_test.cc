#include "gtest/gtest.h"
#include "EchoRequestHandler.h"
#include <boost/lexical_cast.hpp>
#include <string>

class EchoHandlerTest : public ::testing::Test {
protected:
    std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                        std::string requestURI = "/echo/sample.html",
                                                        std::string httpVersion = "HTTP/1.1",
                                                        std::vector<std::string> headers = {},
                                                        std::string body = "") {
        std::string httpRequestString = method + std::string(" ") + requestURI + std::string(" ") + \
            httpVersion + std::string("\r\n");
        for(int i = 0; i < headers.size() ; i++) {
        httpRequestString += headers[i] + "\r\n";
        }
        httpRequestString += "\r\n" + body;
        return httpRequestString;
    }

    void makeRequestWithSpecifiedFields(http::request<http::string_body>&req, std::string method = "GET", 
                                            std::string requestURI = "/echo/sample.html",
                                            std::vector<std::string> headers = {},
                                            std::string body = "") {
        if (method == "GET") {
            req.method(http::verb::get);
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

    std::string getExpectedResponse(std::string reqStr) {
        std::string top = "HTTP/1.1 200 OK\r\n";
        std::string type = "Content-Type: text/plain\r\n";
        std::string len = "Content-Length: " + std::to_string(reqStr.length()) + "\r\n\r\n";
        return top + type + len + reqStr;
    }

    std::string responseToString(http::response<http::string_body>res) {
        std::string res_str;
        std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
        res_str += strHeaders;
        res_str += res.body();
        return res_str;
    }
};

TEST_F(EchoHandlerTest, ValidEchoRequest) {
    std::string method = "GET";
    std::string requestURI = "/echo";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);
    EchoRequestHandler* handler = new EchoRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(EchoHandlerTest, InvalidMethod) {
    std::string method = "sjdkf";
    std::string requestURI = "/echo";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);
    EchoRequestHandler* handler = new EchoRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), "HTTP/1.1 400 Bad Request\r\n\r\n");
}