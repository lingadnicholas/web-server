#include "gtest/gtest.h"
#include "BlockRequestHandler.h"
#include <boost/lexical_cast.hpp>


class BlockHandlerTest : public ::testing::Test {
protected:
    std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                        std::string requestURI = "/sleep",
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
                                            std::string requestURI = "/sleep",
                                            std::string reqStr = "") {
        if (method == "GET") {
            req.method(http::verb::get);
        }
        req.target(requestURI);
        req.body() = reqStr;
    }

    std::string getExpectedResponse(std::string reqStr, std::string sleepTime) {
        std::string res_msg = "200 OK. Sleep " + sleepTime + "s\r\n";

        std::string top = "HTTP/1.1 200 OK\r\n";
        std::string type = "Content-Type: text/plain\r\n";
        std::string len = "Content-Length: " + std::to_string(res_msg.length()) + "\r\n\r\n";
        return top + type + len + res_msg;
    }

    std::string responseToString(http::response<http::string_body>res) {
        std::string res_str;
        std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
        res_str += strHeaders;
        res_str += res.body();
        return res_str;
    }
};


TEST_F(BlockHandlerTest, ValidSleepRequestWithBasePath) {
    std::string method = "GET";
    std::string requestURI = "/sleep";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    BlockRequestHandler* handler = new BlockRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr, "1"));
}

TEST_F(BlockHandlerTest, ValidSleepRequestWithValidTime) {
    std::string method = "GET";
    std::string requestURI = "/sleep/2";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    BlockRequestHandler* handler = new BlockRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr, "2"));
}

TEST_F(BlockHandlerTest, ValidSleepRequestWithInvalidTime) {
    std::string method = "GET";
    std::string requestURI = "/sleep/abc";
    std::string httpVersion = "HTTP/1.1";
    std::vector<std::string> headers = {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", "Accept-Language: en-us"};
    std::string body = "";
    std::string reqStr = makeRequestStringWithSpecifiedFields(method, requestURI, httpVersion, headers, body);

    http::request<http::string_body>req;
    makeRequestWithSpecifiedFields(req,method,requestURI,reqStr);
    BlockRequestHandler* handler = new BlockRequestHandler();
    
    http::response<http::string_body>res;
    handler->handleRequest(req,res);

    ASSERT_EQ(responseToString(res), getExpectedResponse(reqStr, "1"));
}