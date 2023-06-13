#include "gtest/gtest.h"
#include "RequestHandler.h"
#include "ApiRequestHandler.h"
#include <string>
#include "config_parser.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

class ApiHandlerFixture : public ::testing::Test {
  public:
    ApiHandlerFixture() {
        root = "../crud_testing"; // must be = to root in new_format_config_for_testing
        base_uri = "/api"; // must be = to location in new_format_config_for_testing
        boost::filesystem::remove_all(root);
        boost::filesystem::path root_path(root);
        if (!boost::filesystem::exists(root_path)) {
            boost::filesystem::create_directories(root_path);
        }
        bool success = config_parser.Parse("new_format_config_for_testing", &config);
        config.populateHdlrMap(hdlrMap);
    }
  protected:
    std::string root;
    std::string base_uri;
    std::map<std::string, std::vector<int>> entity_ids;
    NginxConfigParser config_parser;
    NginxConfig config;
    std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
};

// Bad request if the method is not GET, PUT, POST, DELETE
TEST_F(ApiHandlerFixture, BadRequest) {
    http::request<http::string_body> request;
    request.target("/api/Shoes");
    request.method(http::verb::patch);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
}

TEST_F(ApiHandlerFixture, InvalidNoEntity) {
    boost::filesystem::path dir_path(root + "/unit_test");
    // cleanup first in case files exist
    boost::filesystem::remove_all(dir_path);
    http::request<http::string_body> request;
    request.target("/api/");
    request.method(http::verb::post);
    request.body() = "\'{\"name\":\"John\", \"age\":30, \"car\":null}\'";
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    EXPECT_EQ(entity_ids.size(), 0);
}

TEST_F(ApiHandlerFixture, InvalidBadURI) {
    http::request<http::string_body> request;
    request.target("/skdjfnksdj/");
    request.method(http::verb::post);
    request.body() = "\'{\"name\":\"John\", \"age\":30, \"car\":null}\'";
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    EXPECT_EQ(entity_ids.size(), 0);
}

TEST_F(ApiHandlerFixture, PostInvalidDirectory) { 
    http::request<http::string_body> request;
    request.target("/api/Shoes/Books"); 
    request.method(http::verb::post);
    request.body() = "\'{\"name\":\"John\", \"age\":30, \"car\":null}\'";
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    EXPECT_EQ(entity_ids.size(), 0);
}

TEST_F(ApiHandlerFixture, PostRequest) {    
    http::request<http::string_body> request;
    request.target("/api/Shoes"); 
    request.method(http::verb::post);
    request.body() = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status1 = api_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/api/Shoes");
    request2.method(http::verb::post);
    request2.body() = "{\"name\":\"Jeff\", \"age\":32, \"car\":null}";
    http::response<http::string_body> response2;
    ApiRequestHandler api_handler2(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status2 = api_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();

    http::request<http::string_body> request3; 
    request3.target("/api/Books"); 
    request3.method(http::verb::post);
    request3.body() = "{\"name\":\"Jessie\", \"age\":45, \"car\":null}";
    http::response<http::string_body> response3;
    ApiRequestHandler api_handler3(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status3 = api_handler3.handleRequest(request3, response3);
    std::string body3 = response3.body();
    
    EXPECT_EQ(HTTP_STATUS_CREATED, status1);
    EXPECT_EQ(HTTP_STATUS_CREATED, status2);
    EXPECT_EQ(HTTP_STATUS_CREATED, status3);
    std::stringstream buffer;
    std::stringstream buffer2;
    std::stringstream buffer3;
    std::ifstream t(root + "/Shoes/1");
    std::ifstream t2(root + "/Shoes/2");
    std::ifstream t3(root + "/Books/1");
    buffer << t.rdbuf();
    buffer2 << t2.rdbuf();
    buffer3 << t3.rdbuf();
    EXPECT_EQ(buffer.str(), "{\"name\":\"John\", \"age\":30, \"car\":null}");
    EXPECT_EQ(buffer2.str(), "{\"name\":\"Jeff\", \"age\":32, \"car\":null}");
    EXPECT_EQ(buffer3.str(), "{\"name\":\"Jessie\", \"age\":45, \"car\":null}");
    std::string rep1 = "{\"id\": 1}";
    std::string rep2 = "{\"id\": 2}";
    std::string rep3 = "{\"id\": 1}";
    EXPECT_EQ(body1, rep1);
    EXPECT_EQ(body2, rep2);
    EXPECT_EQ(body3, rep3);
    boost::filesystem::remove_all(root + "/Shoes");
    boost::filesystem::remove_all(root + "/Books");
}


TEST_F(ApiHandlerFixture, PostWithDeleteds) { 
    std::vector<int> shoes_ids = {3};
    std::vector<int> books_ids = {1,2,4};
    entity_ids["Shoes"] = shoes_ids;
    entity_ids["Books"] = books_ids;
    http::request<http::string_body> request;
    request.target("/api/Shoes");
    request.method(http::verb::post);
    request.body() = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status1 = api_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/api/Books");
    request2.method(http::verb::post);
    request2.body() = "{\"name\":\"Jeff\", \"age\":32, \"car\":null}";
    http::response<http::string_body> response2;
    ApiRequestHandler api_handler2(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status2 = api_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();

    http::request<http::string_body> request3;
    request3.target("/api/Books");
    request3.method(http::verb::post);
    request3.body() =  "{\"name\":\"Jessie\", \"age\":45, \"car\":null}";
    http::response<http::string_body> response3;
    ApiRequestHandler api_handler3(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status3 = api_handler3.handleRequest(request3, response3); 
    std::string body3 = response3.body();
    
    EXPECT_EQ(HTTP_STATUS_CREATED, status1);
    EXPECT_EQ(HTTP_STATUS_CREATED, status2);
    EXPECT_EQ(HTTP_STATUS_CREATED, status3);
    std::stringstream buffer;
    std::stringstream buffer2;
    std::stringstream buffer3;
    std::ifstream t(root + "/Shoes/1");
    std::ifstream t2(root + "/Books/3");
    std::ifstream t3(root + "/Books/5");
    buffer << t.rdbuf();
    buffer2 << t2.rdbuf();
    buffer3 << t3.rdbuf();
    EXPECT_EQ(buffer.str(), "{\"name\":\"John\", \"age\":30, \"car\":null}");
    EXPECT_EQ(buffer2.str(), "{\"name\":\"Jeff\", \"age\":32, \"car\":null}");
    EXPECT_EQ(buffer3.str(), "{\"name\":\"Jessie\", \"age\":45, \"car\":null}");
        std::string rep1 = "{\"id\": 1}";
    std::string rep2 = "{\"id\": 3}";
    std::string rep3 = "{\"id\": 5}";
    EXPECT_EQ(body1, rep1);
    EXPECT_EQ(body2, rep2);
    EXPECT_EQ(body3, rep3);
    boost::filesystem::remove_all(root + "/Shoes");
    boost::filesystem::remove_all(root + "/Books");
}

TEST_F(ApiHandlerFixture, PostWithExistingFiles) {
    boost::filesystem::create_directories(root + "/Shoes");
    std::ostringstream oss;
    oss << "{\"name\":\"John\", \"age\":30, \"car\":null}";
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(root + "/Shoes/2");
    file << body;
    file.close();
    
    std::ostringstream oss2;
    oss2 << "{\"name\":\"Jeff\", \"age\":32, \"car\":null}";
    body = oss2.str();
    oss2.clear();
    std::ofstream file2(root + "/Shoes/1");
    file2 << body;
    file2.close();

    std::ostringstream oss3;
    oss3 << "{\"name\":\"Jessie\", \"age\":45, \"car\":null}";
    body = oss3.str();
    oss3.clear();
    std::ofstream file3(root + "/Shoes/3");
    file3 << body;
    file3.close();

    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    
    std::vector<int> ids = {1,2,3};
    EXPECT_EQ(entity_ids["Shoes"], ids);
    boost::filesystem::remove_all(root + "/Shoes");
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteInvalidPath) {
    http::request<http::string_body> request;
    request.target("/api/invalid");
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteNotInPathCounts) { 
    http::request<http::string_body> request;
    request.target("/api/invalid/1"); 
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteOnDirectory) { 
    http::request<http::string_body> request; 
    boost::filesystem::path dir_path(root + "/test");
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::path final_path(root + "/test/dir");
    boost::filesystem::create_directories(final_path);
    request.target("/api/test/dir");
    request.method(http::verb::delete_);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    // cleanup
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteIDNotInt) { 
    http::request<http::string_body> request; 
    boost::filesystem::path dir_path(root);
    boost::filesystem::create_directories(dir_path);
    // cleanup first in case files exist 
    boost::filesystem::remove_all(dir_path); 
    // Create file: code from https://stackoverflow.com/questions/30029343/how-do-i-create-a-file-with-boost-filesystem-without-opening-it 
    boost::filesystem::ofstream(root + "/abc");
    request.target("/api/abc");
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    // cleanup
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteIDNotInPathCounts) { 
    http::request<http::string_body> request; 
    boost::filesystem::path dir_path(root + "/unit");
    boost::filesystem::create_directories(dir_path);
    // cleanup first in case files exist 
    boost::filesystem::remove_all(dir_path); 
    // Create file: code from https://stackoverflow.com/questions/30029343/how-do-i-create-a-file-with-boost-filesystem-without-opening-it 
    boost::filesystem::ofstream(root + "/unit/1");
    request.target("/api/unit/1");
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    // cleanup 
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, DeleteValidRequest) {     
    http::request<http::string_body> request; 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist 
    boost::filesystem::remove_all(dir_path);
    boost::filesystem::create_directories(dir_path);
    // Create file: code from https://stackoverflow.com/questions/30029343/how-do-i-create-a-file-with-boost-filesystem-without-opening-it 
    boost::filesystem::ofstream(root + "/unit/1");
    request.target("/api/unit/1");
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(entity_ids["unit"].size(), 0);
    // cleanup 
    boost::filesystem::remove_all(root);
}

// If we have the map for unit_test to a vector of {1, 2, 3}
// and we delete 2 and post again,
// The POST should have an ID of 2. 
TEST_F(ApiHandlerFixture, DeleteThenPost) { 
    http::request<http::string_body> request; 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist
    boost::filesystem::remove_all(dir_path);
    boost::filesystem::create_directories(dir_path);
    std::vector<int> ids = {1, 3};
    entity_ids["unit"] = ids;
    // Create file: code from https://stackoverflow.com/questions/30029343/how-do-i-create-a-file-with-boost-filesystem-without-opening-it 
    boost::filesystem::ofstream(root + "/unit/2");
    request.target("/api/unit/2");
    request.method(http::verb::delete_);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(entity_ids["unit"].size(), 2);
    // POST 
    http::request<http::string_body> request2;
    request2.target("/api/unit");
    request2.method(http::verb::post);
    request2.body() = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::response<http::string_body> response2;
    ApiRequestHandler api_handler2(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status2 = api_handler2.handleRequest(request2, response2);
    EXPECT_EQ(HTTP_STATUS_CREATED, status2);
    // Check that 2 is in entity_ids 
    std::map<std::string, std::vector<int>>::iterator map_it;
    map_it = entity_ids.find("unit");
    
    std::vector<int>::iterator vector_it; 
    vector_it = find(map_it->second.begin(), map_it->second.end(), 2); 
    EXPECT_NE(vector_it, map_it->second.end());
    // cleanup 
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, GETValidFile) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty file
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    http::request<http::string_body> request;
    request.target("/api/unit/1"); 
    request.method(http::verb::get);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_OK, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, GETInvalidFile) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty file
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    http::request<http::string_body> request;
    request.target("/api/unit/2"); // FILE doesn't exist
    request.method(http::verb::get);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_NOT_FOUND, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, GETAfterPOST) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty file
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    // POST
    std::string body = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::request<http::string_body> request2; 
    request2.target("/api/unit"); 
    request2.method(http::verb::post);
    request2.body() = body;
    http::response<http::string_body> response2;
    ApiRequestHandler api_handler2(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status2 = api_handler2.handleRequest(request2, response2);
    EXPECT_EQ(HTTP_STATUS_CREATED, status2);
    // GET
    http::request<http::string_body> request;
    request.target("/api/unit/1");
    request.method(http::verb::get);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(body, response.body());
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, LISTValidDir) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    boost::filesystem::ofstream(root + "/unit/2");
    boost::filesystem::ofstream(root + "/unit/3");
    std::string list = "[1,2,3]";
    http::request<http::string_body> request; 
    request.target("/api/unit");
    request.method(http::verb::get);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(list, response.body());
    boost::filesystem::remove_all(root);
}
TEST_F(ApiHandlerFixture, LISTInvalidDir) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty file
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    http::request<http::string_body> request; 
    request.target("/api/unit2"); // DIR doesn't exist
    request.method(http::verb::get);
    http::response<http::string_body> response; 
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, LISTAfterDELETE) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    boost::filesystem::ofstream(root + "/unit/2");
    boost::filesystem::ofstream(root + "/unit/3");
    std::string list = "[1,2,3]";
    // LIST before DELETE
    http::request<http::string_body> request;
    request.target("/api/unit");
    request.method(http::verb::get);
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(list, response.body());
    // DELETE
    http::request<http::string_body> request2;
    request2.target("/api/unit/1");
    request2.method(http::verb::delete_);
    http::response<http::string_body> response2;
    ApiRequestHandler api_handler2(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status2 = api_handler2.handleRequest(request2, response2);
    EXPECT_EQ(HTTP_STATUS_OK, status2);
    std::string list2 = "[2,3]"; // 1 was removed, should no longer be outputted by LIST
    // LIST after DELETE
    http::request<http::string_body> request3; 
    request3.target("/api/unit");
    request3.method(http::verb::get);
    http::response<http::string_body> response3;
    ApiRequestHandler api_handler3(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status3 = api_handler3.handleRequest(request3, response3);
    EXPECT_EQ(HTTP_STATUS_OK, status3);
    EXPECT_EQ(list2, response3.body());
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, PUTValidFile) {
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path);
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    std::string body = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::request<http::string_body> request;
    request.target("/api/unit/1");
    request.method(http::verb::put);
    request.body() = body;
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_OK, status);
    EXPECT_EQ(body, response.body());
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, PUTInvalidFile) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    std::string body = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::request<http::string_body> request;
    request.target("/api/unit/2"); // FILE doesn't exist
    request.method(http::verb::put);
    request.body() = body;
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, PUTDirectory) { 
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path); 
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    std::string body = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::request<http::string_body> request;
    request.target("/api/unit"); // FILE is directory therefore invalid
    request.method(http::verb::put);
    request.body() = body;
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, hdlrMap[base_uri].second, entity_ids);
    int status = api_handler.handleRequest(request, response); 
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    boost::filesystem::remove_all(root);
}

TEST_F(ApiHandlerFixture, ApiHandlerInvalidConfig) {
    boost::filesystem::path dir_path(root + "/unit");
    // cleanup first in case files exist AND create empty files
    boost::filesystem::remove_all(dir_path);
    boost::filesystem::create_directories(dir_path);
    boost::filesystem::ofstream(root + "/unit/1");
    std::string body = "{\"name\":\"John\", \"age\":30, \"car\":null}";
    http::request<http::string_body> request;
    request.target("/api/unit/1");
    request.method(http::verb::put);
    request.body() = body;
    http::response<http::string_body> response;
    ApiRequestHandler api_handler(base_uri, nullptr, entity_ids);
    int status = api_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_INTERNAL_SERVER_ERROR, status);
    boost::filesystem::remove_all(root);
}
