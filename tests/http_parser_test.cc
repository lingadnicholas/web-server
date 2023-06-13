#include "gtest/gtest.h"
#include "HttpParser.h"
#include <boost/lexical_cast.hpp>

class HttpParserTest : public ::testing::Test {
    protected:
        HttpParser parser;
        std::string makeRequestString(std::string method = "GET",
                                std::string requestURI = "/static1/sample.html",
                                std::string httpVersion = "HTTP/1.1",
                                std::vector<std::string> headers = {},
                                std::string body = "") {
            std::string httpRequestString = method + std::string(" ") + \
                                            requestURI + std::string(" ") + \
                                            httpVersion + std::string("\r\n");
            for(int i = 0; i < headers.size() ; i++) {
                httpRequestString += headers[i] + "\r\n";
            }
            httpRequestString += "\r\n" + body;
            return httpRequestString;
	}
};

TEST_F(HttpParserTest, getResponseWorks) {
    http::response<http::string_body> res;
    res.result(400);
    res.reason("Bad Request");
    std::string res_str = parser.getResponse(res);
    ASSERT_EQ(res_str, "HTTP/1.1 400 Bad Request\r\n\r\n");
}

TEST_F(HttpParserTest, getFieldsBasicGet) {
    std::string request_string = makeRequestString();
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "GET");
}

TEST_F(HttpParserTest, getFieldsBasicPut) {
    std::string request_string = makeRequestString("PUT");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "PUT");
}


TEST_F(HttpParserTest, getFieldsBasicPost) {
    std::string request_string = makeRequestString("POST");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "POST");
}

TEST_F(HttpParserTest, getFieldsBasicDelete) {
    std::string request_string = makeRequestString("DELETE");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "DELETE");
}

TEST_F(HttpParserTest, getFieldsBasicHead) {
    std::string request_string = makeRequestString("HEAD");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "HEAD");
}

TEST_F(HttpParserTest, getFieldsBasicConnect) {
    std::string request_string = makeRequestString("CONNECT");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "CONNECT");
}

TEST_F(HttpParserTest, getFieldsBasicTrace) {
    std::string request_string = makeRequestString("TRACE");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "TRACE");
}

TEST_F(HttpParserTest, getFieldsBasicOptions) {
    std::string request_string = makeRequestString("OPTIONS");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "OPTIONS");
}

TEST_F(HttpParserTest, getFieldsReqMethodInvalid) {
    std::string request_string = makeRequestString("LALALA"); // LALALA is not a valid http request method
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.method_string(), "");
}

TEST_F(HttpParserTest, getFieldsWithHeadersAndBody) {
    std::string request_string = makeRequestString("GET", "/static1/text/sample.html", "HTTP/1.1", 
                                {"User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT", 
                                 "Accept-Language: en-us"}, 
                                "This is body");
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "GET");
    ASSERT_EQ(req.body(), "This is body");
    std::string const strHeaders = boost::lexical_cast<std::string>(req.base());
    std::string head = strHeaders.substr(strHeaders.find("User"));
    ASSERT_EQ(head, "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT\r\nAccept-Language: en-us\r\n\r\n");
}

TEST_F(HttpParserTest, getFieldsReqBadSyntaxAfterHTTPVersion) {
    std::string request_string = "GET /static1/text/sample.html HTTP/1.1\r";
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "GET");
    ASSERT_EQ(req.body(), "");
    std::string const strHeaders = boost::lexical_cast<std::string>(req.base());
    ASSERT_EQ(strHeaders, "GET /static1/text/sample.html HTTP/1.1\r\n\r\n");
}

TEST_F(HttpParserTest, getFieldsReqBadSyntaxAfterHeaderLine) {
    std::string request_string = "GET /static1/text/sample.html HTTP/1.1\r\nAccept-Language: en-us\r";
    http::request<http::string_body> req;
    parser.getFields(request_string, req);
    ASSERT_EQ(req.version(), 11);
    ASSERT_EQ(req.method_string(), "GET");
    ASSERT_EQ(req.body(), "");
    std::string const strHeaders = boost::lexical_cast<std::string>(req.base());
    ASSERT_EQ(strHeaders, "GET /static1/text/sample.html HTTP/1.1\r\n\r\n");
}

