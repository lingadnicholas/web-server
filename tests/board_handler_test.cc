#include "gtest/gtest.h"
#include "RequestHandler.h"
#include "BoardRequestHandler.h"
#include <string>
#include <numeric>
#include "config_parser.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

class BoardHandlerFixture : public ::testing::Test {
  public:
    BoardHandlerFixture() {
        root = "../board_testing"; // must be = to root in new_format_config_for_testing
        base_uri = "/board"; // must be = to location in new_format_config_for_testing
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
    std::map<std::string, std::vector<int>> board_notes;
    std::map<std::string, int> board_pins;

    NginxConfigParser config_parser;
    NginxConfig config;
    std::map<std::string, std::pair<std::string, NginxConfig*>> hdlrMap;
};

// Bad request if the method is not GET, PUT, POST, DELETE
TEST_F(BoardHandlerFixture, BadRequest) {
    http::request<http::string_body> request;
    request.target("/board/pretend_this_board_exists");
    request.method(http::verb::patch);
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
}

TEST_F(BoardHandlerFixture, InvalidBadURI) {
    http::request<http::string_body> request;
    request.target("/skdjfnksdj/");
    request.method(http::verb::post);
    request.body() = "\'{\"name\":\"John\", \"age\":30, \"car\":null}\'";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_BAD_REQUEST, status);
    EXPECT_EQ(board_notes.size(), 0);
    EXPECT_EQ(board_pins.size(), 0); 
}

TEST_F(BoardHandlerFixture, ValidBoardFormGETRequest) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::get);
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_OK);
}

/*
// This is commented out because it currently returns 400 error since it's not yet implemented
TEST_F(BoardHandlerFixture, ValidBoardFormPOSTRequest) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=update-note\r\n"
                     "board-ID=crazyboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=1\r\n"
                     "note-text=Hello world\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_OK);
}
*/

TEST_F(BoardHandlerFixture, BadConfigRootMissingResultsIn500Error) {
    bool success = config_parser.Parse("board_hdlr_test_badconfig1", &config);
    config.populateHdlrMap(hdlrMap);
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    http::request<http::string_body> request;
    request.method(http::verb::post);
    http::response<http::string_body> response;
    int status = board_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_INTERNAL_SERVER_ERROR, status);
}

TEST_F(BoardHandlerFixture, BadConfigRootUsedDifferentKeywordResultsIn500Error) {
    bool success = config_parser.Parse("board_hdlr_test_badconfig2", &config);
    config.populateHdlrMap(hdlrMap);
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    http::request<http::string_body> request;
    request.method(http::verb::post);
    http::response<http::string_body> response;
    int status = board_handler.handleRequest(request, response);
    EXPECT_EQ(HTTP_STATUS_INTERNAL_SERVER_ERROR, status);
}

TEST_F(BoardHandlerFixture, PINOnlyThreeDigits) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=update-note\r\n"
                     "board-ID=crazyboard\r\n"
                     "board-PIN=123\r\n"
                     "note-ID=1\r\n"
                     "note-text=Hello world\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_BAD_REQUEST);
}

TEST_F(BoardHandlerFixture, PINHasLetters) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=update-note\r\n"
                     "board-ID=crazyboard\r\n"
                     "board-PIN=123a\r\n"
                     "note-ID=1\r\n"
                     "note-text=Hello world\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_BAD_REQUEST);
}

TEST_F(BoardHandlerFixture, BoardNameHasNonalphanumeric) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=update-note\r\n"
                     "board-ID=crazy board\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=1\r\n"
                     "note-text=Hello world\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_BAD_REQUEST);
}

TEST_F(BoardHandlerFixture, NoteIDIsNotNumeric) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=update-note\r\n"
                     "board-ID=crazyboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=hey\r\n"
                     "note-text=Hello world\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    ASSERT_EQ(status, HTTP_STATUS_BAD_REQUEST);
}

TEST_F(BoardHandlerFixture, CreateBoards) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=create-board\r\n"
                     "board-ID=goofyboard\r\n"
                     "board-PIN=9999\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();
    
    std::stringstream buffer;
    std::stringstream buffer2;
    std::ifstream t(root + "/broboard/pin");
    std::ifstream t2(root + "/goofyboard/pin");
    buffer << t.rdbuf();
    buffer2 << t2.rdbuf();
    std::string rep1 = "Successfully created board with ID: broboard and PIN: 1234\r\n";
    std::string rep2 = "Successfully created board with ID: goofyboard and PIN: 9999\r\n";
    ASSERT_EQ(status, HTTP_STATUS_CREATED);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);
    EXPECT_EQ(buffer.str(), "1234");
    EXPECT_EQ(buffer2.str(), "9999");
    EXPECT_EQ(body1, rep1);
    EXPECT_EQ(body2, rep2);
    std::vector<int> notes = {};
    EXPECT_EQ(board_notes["broboard"], notes);
    EXPECT_EQ(board_notes["goofyboard"], notes);
    EXPECT_EQ(board_pins["broboard"], 1234);
    EXPECT_EQ(board_pins["goofyboard"], 9999);

    boost::filesystem::remove_all(root + "/broboard");
    boost::filesystem::remove_all(root + "/goofyboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, CreateExistingBoard) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=9999\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();
    
    std::stringstream buffer;
    std::ifstream t(root + "/broboard/pin");
    buffer << t.rdbuf();
    std::string rep1 = "Successfully created board with ID: broboard and PIN: 1234\r\n";
    std::string rep2 = "200 OK : board already exists\r\n";
    ASSERT_EQ(status, HTTP_STATUS_CREATED);
    ASSERT_EQ(status2, HTTP_STATUS_OK);
    EXPECT_EQ(buffer.str(), "1234");
    EXPECT_EQ(body1, rep1);
    EXPECT_EQ(body2, rep2);
    std::vector<int> notes = {};
    EXPECT_EQ(board_notes["broboard"], notes);
    EXPECT_EQ(board_pins["broboard"], 1234);

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, InstallExistingFiles) {
    boost::filesystem::create_directories(root + "/crazyboard");
    boost::filesystem::create_directories(root + "/broboard");
    std::ostringstream oss;
    oss << "1234";
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(root + "/crazyboard/pin");
    file << body;
    file.close();
    
    std::ostringstream oss2;
    oss2 << "1111";
    body = oss2.str();
    oss2.clear();
    std::ofstream file2(root + "/broboard/pin");
    file2 << body;
    file2.close();

    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    
    std::vector<int> notes = {};
    EXPECT_EQ(board_notes["crazyboard"], notes);
    EXPECT_EQ(board_notes["broboard"], notes);
    EXPECT_EQ(board_pins["crazyboard"], 1234);
    EXPECT_EQ(board_pins["broboard"], 1111);
    boost::filesystem::remove_all(root + "/crazyboard");
    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, ValidCreateNote) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=create-board\r\n"
                     "board-ID=goofyboard\r\n"
                     "board-PIN=9999\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();
    
    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=post-note\r\n"
                     "board-ID=goofyboard\r\n"
                     "board-PIN=9999\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response3;

    http::request<http::string_body> request4;
    request4.target("/board");
    request4.method(http::verb::post);
    request4.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response4;


    http::request<http::string_body> request5;
    request5.target("/board");
    request5.method(http::verb::post);
    request5.body() = "crud-type=post-note\r\n"
                     "board-ID=goofyboard\r\n"
                     "board-PIN=9999\r\n"
                     "note-ID=\r\n"
                     "note-text=abcde\r\n";
    http::response<http::string_body> response5;


    std::vector<int> notes1 = {1};
    std::vector<int> notes2 = {1, 2};

    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    std::string body3 = response3.body();

    EXPECT_EQ(board_notes["goofyboard"], notes1);
    BoardRequestHandler board_handler4(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status4 = board_handler4.handleRequest(request4, response4);
    std::string body4 = response4.body();
    EXPECT_EQ(board_notes["broboard"], notes1);

    BoardRequestHandler board_handler5(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status5 = board_handler5.handleRequest(request5, response5);
    std::string body5 = response5.body();
    EXPECT_EQ(board_notes["goofyboard"], notes2);
    EXPECT_EQ(board_notes["broboard"], notes1);
    EXPECT_EQ(status3, HTTP_STATUS_CREATED);
    EXPECT_EQ(status4, HTTP_STATUS_CREATED);
    EXPECT_EQ(status5, HTTP_STATUS_CREATED);

    boost::filesystem::remove_all(root + "/broboard");
    boost::filesystem::remove_all(root + "/goofyboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, PostNoteButBoardNotExist) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=post-note\r\n"
                     "board-ID=nonexistant\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();
    EXPECT_EQ(status, HTTP_STATUS_BAD_REQUEST);
    EXPECT_EQ(body1, "400 Bad Request : createNote : Board does not exist\r\n"); 
}

TEST_F(BoardHandlerFixture, CreateNoteButMaxNotes) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;


    std::vector<int> temp( 100 );
    std::iota( temp.begin(), temp.end(), 1 ); // fills from 1-100
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();
    
    http::request<http::string_body> request2;
    board_notes["broboard"] = temp; 
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abc\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    std::string body2 = response2.body();

    EXPECT_EQ(status2, HTTP_STATUS_BAD_REQUEST);
    EXPECT_EQ(body2, "400 Bad Request : createNote : Board is at maximum of 100 notes\r\n"); 

    board_pins.clear(); 
    board_notes.clear();    
    boost::filesystem::remove_all(root + "/broboard");
}

TEST_F(BoardHandlerFixture, CreateNoteButInvalidPIN) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;

    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();
    
    http::request<http::string_body> request2;
    http::response<http::string_body> response2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1233\r\n"
                     "note-ID=\r\n"
                     "note-text=abc\r\n";
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler.handleRequest(request2, response2);
    std::string body2 = response2.body();

    EXPECT_EQ(status2, HTTP_STATUS_BAD_REQUEST);
    EXPECT_EQ(body2, "400 Bad Request : createNote : Invalid PIN received: 1233\r\n"); 

    board_pins.clear(); 
    board_notes.clear();    
    boost::filesystem::remove_all(root + "/broboard");
}

TEST_F(BoardHandlerFixture, CreateNoteLongerThanMaxLen) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;

    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();


    std::string s(281, 'a'); // string greater than max length 
    http::request<http::string_body> request2;
    http::response<http::string_body> response2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=" + s + "\r\n";
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler.handleRequest(request2, response2);
    std::string body2 = response2.body();

    EXPECT_EQ(status2, HTTP_STATUS_BAD_REQUEST);
    EXPECT_EQ(body2, "400 Bad Request : createNote : Note too long. Max is 280 characters\r\n"); 

    board_pins.clear(); 
    board_notes.clear();    
    boost::filesystem::remove_all(root + "/broboard");
}

TEST_F(BoardHandlerFixture, PostWithDeletedNotes) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    std::vector<int> temp{1, 2, 5}; 
    board_notes["broboard"] = temp; 

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler.handleRequest(request2, response2);
    std::string body2 = response2.body();

    EXPECT_EQ(status2, HTTP_STATUS_CREATED); 
    EXPECT_EQ(body2, "{\"id\": 6}");

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, ValidViewBoard) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status1 = board_handler.handleRequest(request, response);
    ASSERT_EQ(status1, HTTP_STATUS_CREATED);

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=view-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    ASSERT_EQ(status3, HTTP_STATUS_OK);

    std::vector<int> notes1 = {1};

    EXPECT_EQ(board_notes["broboard"], notes1);

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, ValidDeleteNote) {
    board_pins.clear();
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status1 = board_handler.handleRequest(request, response);
    ASSERT_EQ(status1, HTTP_STATUS_CREATED);

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=delete-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=0000\r\n"
                     "note-ID=1\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    ASSERT_EQ(status3, HTTP_STATUS_OK);
    std::vector<int> notes = {1};
    EXPECT_EQ(board_notes["broboard"], notes);

    http::request<http::string_body> request4;
    request4.target("/board");
    request4.method(http::verb::post);
    request4.body() = "crud-type=delete-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=1\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response4;
    BoardRequestHandler board_handler4(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status4 = board_handler4.handleRequest(request4, response4);
    ASSERT_EQ(status4, HTTP_STATUS_OK);


    std::vector<int> notes1 = {};

    EXPECT_EQ(board_notes["broboard"], notes1);

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, GetBoards) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status1 = board_handler.handleRequest(request, response);
    ASSERT_EQ(status1, HTTP_STATUS_CREATED);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=get-boards\r\n"
                     "board-ID=\r\n"
                     "board-PIN=\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    ASSERT_EQ(status3, HTTP_STATUS_OK);

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, EmptyGetBoards) {
    board_pins.clear(); 
    board_notes.clear();
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=get-boards\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status1 = board_handler.handleRequest(request, response);
    ASSERT_EQ(status1, HTTP_STATUS_OK);
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, ValidUpdateNote) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=update-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=1\r\n"
                     "note-text=fghi\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    
    ASSERT_EQ(status, HTTP_STATUS_CREATED);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);
    ASSERT_EQ(status3, HTTP_STATUS_OK);
    std::stringstream buffer;
    std::ifstream t(root + "/broboard/1");
    buffer << t.rdbuf();
    std::string rep = "Successfully updated note at board: broboard with ID: 1\r\n";
    EXPECT_EQ(buffer.str(), "fghi");
    EXPECT_EQ(response3.body(), rep);

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, UpdateNonexistentNote) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=update-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=2\r\n"
                     "note-text=fghi\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    
    ASSERT_EQ(status, HTTP_STATUS_CREATED);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);
    ASSERT_EQ(status3, HTTP_STATUS_BAD_REQUEST);
    std::stringstream buffer;
    std::ifstream t(root + "/broboard/1");
    buffer << t.rdbuf();
    EXPECT_EQ(buffer.str(), "abcd");

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}

TEST_F(BoardHandlerFixture, UpdateInvalidPIN) {
    http::request<http::string_body> request;
    request.target("/board");
    request.method(http::verb::post);
    request.body() = "crud-type=create-board\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=\r\n";
    http::response<http::string_body> response;
    BoardRequestHandler board_handler(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status = board_handler.handleRequest(request, response);
    std::string body1 = response.body();

    http::request<http::string_body> request2;
    request2.target("/board");
    request2.method(http::verb::post);
    request2.body() = "crud-type=post-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1234\r\n"
                     "note-ID=\r\n"
                     "note-text=abcd\r\n";
    http::response<http::string_body> response2;
    BoardRequestHandler board_handler2(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status2 = board_handler2.handleRequest(request2, response2);

    http::request<http::string_body> request3;
    request3.target("/board");
    request3.method(http::verb::post);
    request3.body() = "crud-type=update-note\r\n"
                     "board-ID=broboard\r\n"
                     "board-PIN=1111\r\n"
                     "note-ID=1\r\n"
                     "note-text=fghi\r\n";
    http::response<http::string_body> response3;
    BoardRequestHandler board_handler3(base_uri, hdlrMap[base_uri].second, board_notes, board_pins);
    int status3 = board_handler3.handleRequest(request3, response3);
    
    ASSERT_EQ(status, HTTP_STATUS_CREATED);
    ASSERT_EQ(status2, HTTP_STATUS_CREATED);
    ASSERT_EQ(status3, HTTP_STATUS_OK);
    std::stringstream buffer;
    std::ifstream t(root + "/broboard/1");
    buffer << t.rdbuf();
    EXPECT_EQ(buffer.str(), "abcd");
    EXPECT_EQ(response3.body(), "200 OK: Invalid PIN for board, so note cannot be updated");

    boost::filesystem::remove_all(root + "/broboard");
    board_pins.clear(); 
    board_notes.clear();
}