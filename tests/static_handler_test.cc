#include "gtest/gtest.h"
#include "RequestHandler.h"
#include "StaticRequestHandler.h"
#include <string>
#include "config_parser.h"
#include <boost/lexical_cast.hpp>

class StaticHandlerTest : public ::testing::Test {

    protected:
        NginxConfigParser config_parser;
        NginxConfig config;
        std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;

        std::string makeRequestStringWithSpecifiedFields(std::string method = "GET", 
                                                         std::string requestURI = "/static1/sample.html",
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
            if (method == "PUT") {
                req.method(http::verb::put);
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

        std::string makeResponseStringWithSpecifiedFields(std::string httpVer,
                                                          std::string statusPhrase,
                                                          std::vector<std::string> headers,
                                                          std::string body) {
            std::string res = "";
            res += httpVer + " " + statusPhrase + "\r\n";
            for(int i = 0; i < headers.size(); i++) {
                res += headers[i] + "\r\n";
            }
            res += "\r\n" + body;
            return res;
        }

        std::string responseToString(http::response<http::string_body>res) {
            std::string res_str;
            std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
            res_str += strHeaders;
            res_str += res.body();
            return res_str;
        }

};

TEST_F(StaticHandlerTest, BadMethodRequestGets400ErrorResponse) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1/text", hdlrMap["/static1/text"].second);
  std::string method = "LALALA"; // LALALA is not a valid Method 
  std::string requestURI = "/static1/text";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);

  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);
  std::string expectedRes = makeResponseStringWithSpecifiedFields(
    httpVersion, "400 Bad Request", headers, "");
  ASSERT_EQ(responseToString(res), expectedRes);
}

TEST_F(StaticHandlerTest, GetRequestForHtmlFile) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1/text", hdlrMap["/static1/text"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/text/sample.html";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);
  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  ASSERT_EQ(res.body(), "<!DOCTYPE html>\n<html>\n<body>\n<h1>My Heading</h1>\n<p>My paragraph</p>\n</body>\n</html>\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: text/html\r\nContent-Length: 86\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForTxtFile) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1/text", hdlrMap["/static1/text"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/text/file1.txt";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  ASSERT_EQ(res.body(), "The quick brown fox jumps over the lazy dog.\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: text/plain\r\nContent-Length: 45\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForJpgFile) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1/images", hdlrMap["/static1/images"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/images/orange-sun-small.jpg";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: image/jpeg\r\nContent-Length: 1295\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForPngFile) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1/images", hdlrMap["/static1/images"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/images/hehe-kitty.png";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: image/png\r\nContent-Length: 6140\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForZipFile) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/files.zip";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: application/zip\r\nContent-Length: 512\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForFileWithNoFileExtension) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/file";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  ASSERT_EQ(res.body(), "hello\n");
  std::string const strHeaders = boost::lexical_cast<std::string>(res.base());
  std::string head = strHeaders.substr(strHeaders.find("Content"));
  ASSERT_EQ(head, "Content-Type: application/octet-stream\r\nContent-Length: 6\r\n\r\n");
}

TEST_F(StaticHandlerTest, GetRequestForNonexistentFile404Error) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/fileThatDoesNotExist";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 404);
}

TEST_F(StaticHandlerTest, NonGetRequestGetsAnEchoResponseSinceNotYetHandledByServer) {
  bool success = config_parser.Parse("new_format_config_for_testing", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "PUT";
  std::string requestURI = "/static1/file";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 200);
  ASSERT_EQ(res.body(), reqStr);
}

TEST_F(StaticHandlerTest, BadConfigRootMissingResultsIn500Error) {
  bool success = config_parser.Parse("static_hdlr_test_badconfig1", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/file";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 500);
}

TEST_F(StaticHandlerTest, BadConfigRootUsedDifferentKeywordResultsIn500Error) {
  bool success = config_parser.Parse("static_hdlr_test_badconfig2", &config);
  config.populateHdlrMap(hdlrMap);
  StaticRequestHandler handler("/static1", hdlrMap["/static1"].second);
  std::string method = "GET";
  std::string requestURI = "/static1/file";
  std::string httpVersion = "HTTP/1.1";
  std::vector<std::string> headers = {};
  std::string body = "";
  std::string reqStr = makeRequestStringWithSpecifiedFields(
    method, requestURI, httpVersion, headers, body);
  
  http::request<http::string_body>req;
  makeRequestWithSpecifiedFields(req,method,requestURI,headers,body);

  http::response<http::string_body>res;
  handler.handleRequest(req,res);

  ASSERT_EQ(res.version(), 11);
  ASSERT_EQ(res.result_int(), 500);
}
