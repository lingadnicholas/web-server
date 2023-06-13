#include "gtest/gtest.h"
#include "HealthRequestHandler.h"
#include <boost/lexical_cast.hpp>
#include <string>

class HealthHandlerTest : public ::testing::Test {
protected:
    std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                        std::string requestURI = "/",
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
                                            std::string requestURI = "/",
                                            std::string reqStr = "") {
        if (method == "GET") {
            req.method(http::verb::get);
        }
        req.target(requestURI);
        req.body() = reqStr;
    }

    std::string getExpectedResponse(std::string reqStr) {
        std::string ok_msg = "OK\r\n";

        std::string top = "HTTP/1.1 200 OK\r\n";
        std::string type = "Content-Type: text/plain\r\n";
        std::string len = "Content-Length: " + std::to_string(ok_msg.length()) + "\r\n\r\n";
        return top + type + len + ok_msg;
    }

    std::string responseToString(http::response<http::string_body>res) {
        std::string res_str;
        std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
        res_str += strHeaders;
        res_str += res.body();
        return res_str;
    }
};

TEST_F(HealthHandlerTest, ValidHealthRequest) {
    std::string method = "GET";
    std::string requestURI = "/health";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    HealthRequestHandler* handler = new HealthRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(HealthHandlerTest, InvalidMethod) {
    std::string method = "呵呵";
    std::string requestURI = "/health";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    HealthRequestHandler* handler = new HealthRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(HealthHandlerTest, InvalidHttp) {
    std::string method = "GET";
    std::string requestURI = "/health";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    
    std::string reqStr = method + std::string(" ") + requestURI + std::string(" ") + httpVersion + std::string("\r");
    for(int i = 0; i < headers.size() ; i++) {
        reqStr += headers[i] + "\r\n";
    }
    reqStr += "\r\n" + body;

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    HealthRequestHandler* handler = new HealthRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(HealthHandlerTest, InvalidHeader) {
    std::string method = "GET";
    std::string requestURI = "/health";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    
    std::string reqStr = method + std::string(" ") + requestURI + std::string(" ") + httpVersion + std::string("\r\n");
    for(int i = 0; i < headers.size() ; i++) {
        reqStr += headers[i] + "\r";
    }
    reqStr += "\r\n" + body;

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    HealthRequestHandler* handler = new HealthRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}