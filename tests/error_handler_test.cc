#include "gtest/gtest.h"
#include "ErrorRequestHandler.h"
#include <boost/lexical_cast.hpp>
#include <string>

class ErrorHandlerTest : public ::testing::Test {
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
        std::string error_msg = "404 Not Found\r\n";

        std::string top = "HTTP/1.1 404 Not Found\r\n";
        std::string type = "Content-Type: text/plain\r\n";
        std::string len = "Content-Length: " + std::to_string(error_msg.length()) + "\r\n\r\n";
        return top + type + len + error_msg;
    }

    std::string responseToString(http::response<http::string_body>res) {
        std::string res_str;
        std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
        res_str += strHeaders;
        res_str += res.body();
        return res_str;
    }
};

TEST_F(ErrorHandlerTest, ValidErrorRequestWithBasePath) {
    std::string method = "GET";
    std::string requestURI = "/";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    ErrorRequestHandler* handler = new ErrorRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(ErrorHandlerTest, ValidErrorRequestWithIncorrectPath) {
    std::string method = "GET";
    std::string requestURI = "/blah/blahh";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    ErrorRequestHandler* handler = new ErrorRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(ErrorHandlerTest, InvalidMethod) {
    std::string method = "呵呵";
    std::string requestURI = "/";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "This is some very interesting message body text!";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    ErrorRequestHandler* handler = new ErrorRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(ErrorHandlerTest, InvalidHttp) {
    std::string method = "GET";
    std::string requestURI = "/";
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
    ErrorRequestHandler* handler = new ErrorRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}

TEST_F(ErrorHandlerTest, InvalidHeader) {
    std::string method = "GET";
    std::string requestURI = "/";
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
    ErrorRequestHandler* handler = new ErrorRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr));
}