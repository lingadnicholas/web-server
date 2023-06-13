#include <algorithm>
#include "BoardRequestHandler.h"
#include <bits/stdc++.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <ctype.h>
#include <filesystem>
#include <fstream>
#include "log.h"
#include "MIME.h"

// Reference tutorial used for HTML/CSS:
// https://webdesign.tutsplus.com/tutorials/create-a-sticky-note-effect-in-5-easy-steps-with-css3-and-html5--net-13934

#define VIEW_BOARD_INIT_HTML "<!DOCTYPE html><html><body>" \
"<style>" \
"body { margin: 20px auto; font-family: sans-serif; background:tan; color:#fff; }" \
"* { margin: 0; padding: 0; }" \
"h1 { margin: 10px; text-align: center; }" \
"h2 { margin: 10px; font-size: 6rem; color: darkblue; text-align: center; }" \
"h3 { font-size: 1rem; }" \
"p { font-size: 1rem; }" \
"ul,li { list-style: none; }" \
"ul { display: flex; flex-wrap: wrap; justify-content: center; }" \
"ul li #note { text-decoration:none; color:#000; background: rgb(255, 255, 120); display:block; height:10em; width:10em; padding:1em; }" \
"ul li { margin: 1em; }" \
"</style><h1>Bulletin Board - CS130 Assignment 9</h1>"

#define VIEW_BOARD_END_HTML "</body></html>"

BoardRequestHandler :: BoardRequestHandler(const std::string& path,
                                             NginxConfig* config,
                                             std::map<std::string, std::vector<int>>& board_notes,
                                             std::map<std::string, int>& board_pins)
    : RequestHandler(), loc(path), board_notes(board_notes), board_pins(board_pins) {
    LOG_INFO << "BoardRequestHandler :: Constructor\n";
    // parse config in search of root directive
    if (config == nullptr || config->statements_.size() < 1) {
        LOG_ERROR << "BoardRequestHandler :: Constructor : loc=" << path
                  << " has bad config, no statements in block\n";
        badConfig = true;
        return;
    }
    NginxConfigStatement *stmt = config->statements_[0].get(); // assume first statement is root /path; statement
    if (stmt->tokens_.size() != 2 || stmt->tokens_[0] != "root") {
        LOG_ERROR << "BoardRequestHandler :: Constructor : loc=" << path
                  << " has bad config, missing or wrongly named root directive\n";
        badConfig = true;
        return;
    }
    root = stmt->tokens_[1];
    LOG_INFO << "BoardRequestHandler :: Constructor : config is good, loc=" << path << " and root=" << root << "\n";
    badConfig = false;

    // Create root directory if it doesn't already exist
    if (!boost::filesystem::exists(root)) {
        LOG_INFO << "BoardRequestHandler :: Constructor : root path " << root << " does not exist, creating now\n";
        boost::filesystem::create_directories(root);
    }

    // load existing files from the root directory into the board_notes map
    for (auto file : boost::filesystem::recursive_directory_iterator(root)) {
        if (!is_regular_file(file)) {
            LOG_INFO << "BoardRequestHandler :: Constructor : ignoring non-file in crud directory: " <<
                                        absolute(file).string() << std::endl;
            continue;
        }
        boost::filesystem::path file_path = absolute(file);
        std::string file_path_str = file_path.string();
        LOG_INFO << "BoardRequestHandler :: Constructor : file_path_str in root dir = " << file_path_str << "\n";
        int id = 0;
        // check that the filename is an integer
        if (isdigit(file_path_str[file_path_str.length()-1])) {
            id = std::stoi(file_path.stem().string());
        }
        else if (file_path_str.substr(file_path_str.find_last_of("/")+1) == "pin") {
            std::string board = file_path_str.substr(0,file_path_str.find_last_of("/"));
            board = board.substr(board.find_last_of("/")+1);
            std::string path_rel = root + "/" + board + "/pin";

            std::stringstream buffer;
            std::ifstream t(path_rel);
            buffer << t.rdbuf();
            int pin = std::stoi(buffer.str());

            LOG_INFO << "BoardRequestHandler :: Constructor : set board_pins board = " << board << " with pin: " << pin << "\n";
            board_pins[board] = pin;
            continue;
        }
        else {
            LOG_INFO << "BoardRequestHandler :: Constructor : ignoring file with non-integer name";
            continue;
        }
        // remove the id and trailing slash part of the file path
        std::string board = file_path_str.substr(0,file_path_str.find_last_of("/"));
        board = board.substr(board.find_last_of("/")+1);
        std::vector<int> ids = board_notes[board];
        // Only insert if id is NOT in ids (otherwise we get duplicates due to short-lived handlers)
        if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
            ids.push_back(id);
        }
        sort(ids.begin(), ids.end());
        board_notes[board] = ids;
        LOG_INFO << "BoardRequestHandler :: Constructor : set board_notes board = " << board << " with ids:\n";
        for(int i = 0; i < ids.size(); i++) {
            LOG_INFO << "BoardRequestHandler :: Constructor : " << ids[i] << "\n";
        }
    }
}

int BoardRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    LOG_INFO << "BoardRequestHandler :: in handleRequest\n";
    if (badConfig) {
        LOG_INFO << "BoardRequestHandler :: handleRequest : badConfig so returning 500 Internal Server Error\n";
        std::string error_msg = "500 Internal Server Error\r\n";
        int contentLength = error_msg.length();
        std::string contentType = CONTENT_TYPE_TEXT_PLAIN;
        res.set(http::field::content_length, std::to_string(contentLength));
        res.set(http::field::content_type, contentType);
        res.result(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        res.reason(HTTP_REASON_INTERNAL_SERVER_ERROR);
        res.body() = error_msg;
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    std::string requestURI = req.target().to_string();
    
    if (requestURI != loc) {
        LOG_INFO << "BoardRequestHandler :: handleRequest : returning 404 File Not Found because requestURI [" << requestURI << 
                    "] is not equal to handler location [" << loc << "]\n";
        return handleBadRequest(res, "requestURI [" + requestURI + "] is not supported. Handler only supports location [" + loc + "]\n");
    }
    
    http::verb requestMethod = req.method();

    if (requestMethod != http::verb::get && requestMethod != http::verb::post) {
        LOG_INFO << "BoardRequestHandler :: handleRequest : returning 400 Bad Request because requestMethod is neither GET nor POST\n";
        return handleBadRequest(res, "Request method must be GET or POST");
    }

    // If GET request, serve the user a form at the /board (or configured location)
    if (requestMethod == http::verb::get) {
        // for instance, requestURI == "/board", then serve the html form page
        std::string filePath = "../boardForm.html";
        std::ifstream istream(filePath, std::ios::in | std::ios::binary);
        // if file is not found, not readable, or is a directory or other non-regular file type
        if (!istream.good() || !boost::filesystem::is_regular_file(filePath)) {
            LOG_INFO << "BoardRequestHandler :: handleRequest : unable to open ../boardForm.html, therefore, returning 404 File Not Found\n";
            return handleNotFound(res);
	}
        else {
            std::string body((std::istreambuf_iterator<char>(istream)),
                             (std::istreambuf_iterator<char>()));
            int content_length = body.length();
            MIME mime;
            std::string content_type = mime.getContentType("html");
            res.set(http::field::content_type, content_type);
            res.set(http::field::content_length, std::to_string(content_length));
            res.result(HTTP_STATUS_OK);
            res.reason(HTTP_REASON_OK);
            res.body() = body;
            return HTTP_STATUS_OK;
	}
    }

    // If POST request (POST is generated by the form submission), extract data from the req
    BoardRequest boardReq = parseBoardRequest(req.body());

    // return bad request for invalid board name (not alphanumeric)
    // If get-boards, do not check.
    if (boardReq.crudType != "get-boards" && !isValidBoardID(boardReq.boardID)) {
        return handleBadRequest(res, "boardID [" + boardReq.boardID + "] must be all alphanumeric\n");
    }
    LOG_INFO << "BoardRequestHandler :: handleRequest : checked boardID [" + boardReq.boardID + "] is alphanumeric\n";

    // return bad request for invalid PIN format (not 4 digits)
    if (boardReq.crudType == "create-board" || boardReq.crudType == "post-note" 
        || boardReq.crudType == "delete-note" || boardReq.crudType == "update-note") {
        if (!isValidPIN(boardReq.boardPIN)) {
            return handleBadRequest(res, "boardPIN [" + boardReq.boardPIN + "] is not 4 digits\n");
        }
        LOG_INFO << "BoardRequestHandler :: handleRequest : checked boardPIN [" + boardReq.boardPIN + "] is 4 digits\n";
    }

    // return bad request for non numeric noteID
    if (boardReq.crudType == "delete-note" || boardReq.crudType == "update-note") {
        if (!isValidNoteID(boardReq.noteID)) {
            return handleBadRequest(res, "noteID [" + boardReq.noteID + "] is not numeric\n");
	}
    }
    LOG_INFO << "BoardRequestHandler :: handleRequest : checked noteID [" + boardReq.noteID + "] is numeric\n";

    // handle the various crud operations by calling the appropriate handler functions
    if (boardReq.crudType == "create-board") {
        LOG_INFO << "BoardRequestHandler :: handleRequest : calling create new-board\n";
        return createBoard(boardReq, res);
    }
    else if (boardReq.crudType == "view-board") {
        return viewBoard(boardReq, res);
    }
    else if (boardReq.crudType == "get-boards") {
        return getBoards(boardReq, res);
    }
    else if (boardReq.crudType == "post-note") {
        return createNote(boardReq, res);
    }
    else if (boardReq.crudType == "delete-note") {
        return deleteNote(boardReq, res);
    }
    else if (boardReq.crudType == "update-note") {
        return updateNote(boardReq, res);
    }
    else {
        return handleBadRequest(res, "invalid inputs, crudType=" + boardReq.crudType);
    }
    return HTTP_STATUS_INTERNAL_SERVER_ERROR; 
}

int BoardRequestHandler::createBoard(const BoardRequest req, http::response<http::string_body>& res) {
    std::string path = root + "/" + req.boardID;
    boost::system::error_code ec;

    if (boost::filesystem::exists(path)) {
        LOG_INFO << "BoardRequestHandler :: createBoard : board " << req.boardID << " already exists\n";
        std::string msg = "200 OK : board already exists\r\n";
        int contentLength = msg.length();
        std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
        res.set(http::field::content_length, std::to_string(contentLength));
        res.set(http::field::content_type, contentType);
        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);
        res.body() = msg;
        return HTTP_STATUS_OK;
    }
    else 
    {
        boost::filesystem::create_directory(path, ec);
        if (ec) {
            LOG_ERROR << "BoardRequestHandler :: createBoard : error creating directory - " << ec;
            return handleBadRequest(res, "createBoard: error creating directory");
        }

        LOG_INFO << "BoardRequestHandler :: createBoard : creating file with path " << path;
        LOG_INFO << "BoardRequestHandler :: createBoard : creating file with body [" << req.boardPIN << "]\n";
        std::ostringstream oss;
        oss << req.boardPIN;
        std::string body = oss.str();
        oss.clear();
        std::string file_path = path + "/pin";
        std::ofstream file(file_path);
        file << body;
        file.close();

        std::string reply = "Successfully created board with ID: " + req.boardID + " and PIN: " + req.boardPIN + "\r\n";
        res.body() = reply;
        res.set(http::field::content_length, std::to_string(res.body().size()));
        res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
        res.set(http::field::content_location, loc + "/" + req.boardID);
        res.result(HTTP_STATUS_CREATED);
        res.reason(HTTP_REASON_CREATED);

        board_notes[req.boardID] = {};
        board_pins[req.boardID] = std::stoi(req.boardPIN);
        return HTTP_STATUS_CREATED;
    }
}

int BoardRequestHandler::viewBoard(const BoardRequest req, http::response<http::string_body>& res) {
    if (!boardExists(req.boardID)) {
        return handleBadRequest(res, "board [" + req.boardID + "] does not exist");
    }
    std::vector<int> noteIDs = board_notes[req.boardID];
    std::string body = VIEW_BOARD_INIT_HTML;
    body += "<h2>" + req.boardID + "</h2><ul>";
    for (int i = 0; i < noteIDs.size(); i++) {
        int noteID = noteIDs[i];
        std::string filePath = root + "/" + req.boardID + "/" + std::to_string(noteID);
        std::ifstream istream(filePath, std::ios::in | std::ios::binary);
        // if file is not found, not readable, or is a directory or other non-regular file type
        if (!istream.good() || !boost::filesystem::is_regular_file(filePath)) {
            LOG_WARNING << "BoardRequestHandler :: handleRequest : File [" << filePath << "] Not Found but continuing\n";
            continue;
        }
	std::string text((std::istreambuf_iterator<char>(istream)),
                         (std::istreambuf_iterator<char>()));
        // Resource used for the two lines above:
        // https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
        body += "<li><div id=\"note\">";
	body += "<h3>" + std::to_string(noteID) + "</h3>";
        body += "<p>" + text + "</p></div></li>";
    }
    body += "</ul>";
    body += VIEW_BOARD_END_HTML;
    int content_length = body.length();
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_HTML);
    res.set(http::field::content_length, std::to_string(content_length));
    res.result(HTTP_STATUS_OK);
    res.reason(HTTP_REASON_OK);
    res.body() = body;
    return HTTP_STATUS_OK;
}

int BoardRequestHandler::getBoards(const BoardRequest req, http::response<http::string_body>& res) {
    std::string body = VIEW_BOARD_INIT_HTML;
    body += "<h2>Board Names</h2><ul>";

    int boardNum = 1; 
    for (auto i = board_notes.begin(); i != board_notes.end(); i++) {
        body += "<li><div id=\"note\">";
	    body += "<h3>" + std::to_string(boardNum) + "</h3>";
        body += "<p>" + i->first + "</p></div></li>";
        boardNum++;
    
    }
    body += "</ul>";

    if (boardNum == 1) { // no boards yet 
        body += "<li><div id=\"note\">";
	    body += "<h3>" + std::to_string(boardNum) + "</h3>";
        body += "<p>No boards created yet!</p></div></li>";
    }
    body += VIEW_BOARD_END_HTML;
    int content_length = body.length();
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_HTML);
    res.set(http::field::content_length, std::to_string(content_length));
    res.result(HTTP_STATUS_OK);
    res.reason(HTTP_REASON_OK);
    res.body() = body;
    return HTTP_STATUS_OK;
}

int BoardRequestHandler::createNote(const BoardRequest req, http::response<http::string_body>& res) {
    std::string path = root + "/" + req.boardID;
    boost::system::error_code ec;
    const int TWEET_LENGTH = 280; 

    if (!boardExists(req.boardID)) {
        LOG_INFO << "BoardRequestHandler :: createNote : Bad request board does not exist\n"; 
        return handleBadRequest(res, "createNote : Board does not exist"); 
    }
    if (atMaxNotes(req.boardID)) {
        LOG_INFO << "BoardRequestHandler :: createNote : Bad request board is at maximum of 100 notes\n"; 
        return handleBadRequest(res, "createNote : Board is at maximum of 100 notes"); 
    }
    if (!validatePIN(req.boardID, std::stoi(req.boardPIN))) {
        LOG_INFO << "BoardRequestHandler :: createNote : Bad request invalid PIN\n"; 
        return handleBadRequest(res, "createNote : Invalid PIN received: " + req.boardPIN); 
    }
    if (req.noteText.length() > TWEET_LENGTH) {
        LOG_INFO << "BoardRequestHandler :: createNote : Bad request note is longer than 280 chars"; 
        return handleBadRequest(res, "createNote : Note too long. Max is " + std::to_string(TWEET_LENGTH) + " characters"); 
    }

    LOG_INFO << "BoardRequestHandler :: createNote : Getting next ID, create note request is good.\n";
    int id = getNextID(req.boardID); 
    std::string file_path = path + "/" + std::to_string(id);
    LOG_INFO << "BoardRequestHandler :: createNote : creating file with path " << file_path;
    LOG_INFO << "BoardRequestHandler :: createNote : creating file with body [" << req.noteText << "]\n";
    std::ostringstream oss;
    oss << req.noteText;
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(file_path);
    file << body;
    file.close();
    board_notes[req.boardID].push_back(id);

    std::string reply = "{\"id\": " + std::to_string(id) + "}";
    res.body() = reply;
    res.set(http::field::content_length, std::to_string(res.body().size()));
    res.set(http::field::content_type, CONTENT_TYPE_APPL_JSON);
    res.set(http::field::content_location, loc + "/" + req.boardID + "/" + std::to_string(id));
    res.result(HTTP_STATUS_CREATED);
    res.reason(HTTP_REASON_CREATED);
    return HTTP_STATUS_CREATED;
}

int BoardRequestHandler::deleteNote(const BoardRequest req, http::response<http::string_body>& res) {
    if (!boardExists(req.boardID)) {
        LOG_INFO << "BoardRequestHandler :: deleteNote : Bad request board does not exist\n"; 
        return handleBadRequest(res, "deleteNote : Board does not exist"); 
    }
    if (!noteExists(req.boardID, req.noteID)) {
        LOG_INFO << "BoardRequestHandler :: deleteNote : Bad request note does not exist\n";
        return handleBadRequest(res, "deleteNote : Note does not exist");
    }
    std::string filePath = root + "/" + req.boardID + "/pin";
    std::ifstream istream(filePath, std::ios::in | std::ios::binary);
    // if file is not found, not readable, or is a directory or other non-regular file type
    if (!istream.good() || !boost::filesystem::is_regular_file(filePath)) {
        return handleInternalServerError(res, "deleteNote : pin file not readable");
    }
    std::string pin_text((std::istreambuf_iterator<char>(istream)),
                     (std::istreambuf_iterator<char>()));
    // Resource used for the two lines above:
    // https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
    std::string pin = pin_text.substr(0, 4);
    if (req.boardPIN != pin) {
        std::string body = "HTTP STATUS 200 OK: Invalid PIN for board, so note cannot be deleted";
        int content_length = body.length();
        res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
        res.set(http::field::content_length, std::to_string(content_length));
        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);
        res.body() = body;
        return HTTP_STATUS_OK;
    }
    std::string path = root + "/" + req.boardID + "/" + req.noteID;
    int noteID = std::stoi(req.noteID);
    std::vector<int> boardNotes = board_notes[req.boardID];
    if (boost::filesystem::remove(path)) {
        boardNotes.erase(std::remove(boardNotes.begin(), boardNotes.end(), noteID), boardNotes.end());
        board_notes[req.boardID] = boardNotes;
        std::string body = "200 OK : note successfully deleted\n";
        int content_length = body.length();
        res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
        res.set(http::field::content_length, std::to_string(content_length));
        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);
        res.body() = body;
        return HTTP_STATUS_OK;
    }
    return handleInternalServerError(res, "deleteNote : internal failure to delete file");
}

int BoardRequestHandler::updateNote(const BoardRequest req, http::response<http::string_body>& res) {
    if (!boardExists(req.boardID)) {
        LOG_INFO << "BoardRequestHandler :: updateNote : Bad request board does not exist\n"; 
        return handleBadRequest(res, "updateNote : Board does not exist"); 
    }
    if (!noteExists(req.boardID, req.noteID)) {
        LOG_INFO << "BoardRequestHandler :: updateNote : Bad request note does not exist\n";
        return handleBadRequest(res, "updateNote : Note does not exist");
    }
    std::string filePath = root + "/" + req.boardID + "/pin";
    std::ifstream istream(filePath, std::ios::in | std::ios::binary);
    // if file is not found, not readable, or is a directory or other non-regular file type
    if (!istream.good() || !boost::filesystem::is_regular_file(filePath)) {
        return handleInternalServerError(res, "updateNote : pin file not readable");
    }
    std::string pin_text((std::istreambuf_iterator<char>(istream)),
                     (std::istreambuf_iterator<char>()));
    // Resource used for the two lines above:
    // https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
    std::string pin = pin_text.substr(0, 4);
    if (req.boardPIN != pin) {
        std::string body = "200 OK: Invalid PIN for board, so note cannot be updated";
        int content_length = body.length();
        res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
        res.set(http::field::content_length, std::to_string(content_length));
        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);
        res.body() = body;
        return HTTP_STATUS_OK;
    }

    std::string path = root + "/" + req.boardID + "/" + req.noteID;
    boost::filesystem::path boost_path(path);
    // if the file doesn't exist, bad request 
    if (!boost::filesystem::exists(boost_path)) {
        LOG_ERROR << "BoardRequestHandler :: updateNote : path does not exist: " << path << std::endl; 
        return handleBadRequest(res, "updateNote: note does not exist");
    }

    std::string note = req.noteText;
    LOG_INFO << "BoardRequestHandler :: updateNote : modifying file at path: " << path << std::endl;
    std::ostringstream oss;
    oss << note;
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(path);
    file << body;
    file.close();
    // Write PUT response
    res.result(HTTP_STATUS_OK);
    res.body() = "Successfully updated note at board: " + req.boardID + " with ID: " + req.noteID + "\r\n";
    res.set(http::field::content_length, std::to_string(res.body().size()));
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_HTML);
    return HTTP_STATUS_OK;
}

bool BoardRequestHandler :: boardExists(std::string directory) {
    return board_notes.find(directory) != board_notes.end(); 
}
bool BoardRequestHandler :: atMaxNotes(std::string directory) {
    return board_notes[directory].size() >= 100;     
}

bool  BoardRequestHandler :: validatePIN(std::string directory, int PIN) {
    return PIN == board_pins[directory]; 
}

bool BoardRequestHandler :: noteExists(std::string boardID, std::string noteID) {
    if (!boardExists(boardID)) {
        return false;
    }
    if (std::find(board_notes[boardID].begin(), board_notes[boardID].end(), std::stoi(noteID)) == board_notes[boardID].end()) {
        return false;
    }
    return true;
}

// For this implementation of getNextID, we simply keep track of 
// the latest ID (ex: if we have IDs [1, 3, 4], then the next ID will be 5)
// (Note that if the most recent note was deleted, we will simply take that ID)
int BoardRequestHandler :: getNextID(std::string directory) {
    return board_notes[directory].size() >= 1 ? board_notes[directory].back() + 1 : 1; 
}

int BoardRequestHandler::handleBadRequest(http::response<http::string_body>& res, std::string reason) {
    std::string error_msg = "400 Bad Request : " + reason + "\r\n";
    LOG_INFO << "BoardRequestHandler :: handleBadRequest : returning " << error_msg;
    int contentLength = error_msg.length();
    std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
    res.set(http::field::content_length, std::to_string(contentLength));
    res.set(http::field::content_type, contentType);
    res.result(HTTP_STATUS_BAD_REQUEST);
    res.reason(HTTP_REASON_BAD_REQUEST);
    res.body() = error_msg;
    return HTTP_STATUS_BAD_REQUEST;
}

int BoardRequestHandler::handleInternalServerError(http::response<http::string_body>& res, std::string reason) {
    std::string error_msg = "500 Internal Server Error : " + reason + "\r\n";
    LOG_INFO << "BoardRequestHandler :: handleBadRequest : returning " << error_msg;
    int contentLength = error_msg.length();
    std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
    res.set(http::field::content_length, std::to_string(contentLength));
    res.set(http::field::content_type, contentType);
    res.result(HTTP_STATUS_INTERNAL_SERVER_ERROR);
    res.reason(HTTP_REASON_INTERNAL_SERVER_ERROR);
    res.body() = error_msg;
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

int BoardRequestHandler::handleNotFound(http::response<http::string_body>& res) {
    std::string error_msg = "404 Not Found\r\n";
    int contentLength = error_msg.length();
    std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
    res.set(http::field::content_length, std::to_string(contentLength));
    res.set(http::field::content_type, contentType);
    res.result(HTTP_STATUS_NOT_FOUND);
    res.reason(HTTP_REASON_NOT_FOUND);
    res.body() = error_msg;
    return HTTP_STATUS_NOT_FOUND;
}

BoardRequest BoardRequestHandler::parseBoardRequest(std::string body) {
    BoardRequest boardReq;
    std::string crudType = "crud-type=";
    std::string boardID = "board-ID=";
    std::string boardPIN = "board-PIN=";
    std::string noteID = "note-ID=";
    std::string noteText = "note-text=";
    int crudTypeStartIndex = body.find(crudType) + crudType.length();
    int boardIDStartIndex = body.find(boardID) + boardID.length();
    int boardPINStartIndex = body.find(boardPIN) + boardPIN.length();
    int noteIDStartIndex = body.find(noteID) + noteID.length();
    int noteTextStartIndex = body.find(noteText) + noteText.length();
    boardReq.crudType = body.substr(crudTypeStartIndex,
                                    boardIDStartIndex - boardID.length() - crudTypeStartIndex - 2);
    boardReq.boardID = body.substr(boardIDStartIndex,
                                   boardPINStartIndex - boardPIN.length() - boardIDStartIndex - 2);
    boardReq.boardPIN = body.substr(boardPINStartIndex, 
                                    noteIDStartIndex - noteID.length() - boardPINStartIndex - 2);
    boardReq.noteID = body.substr(noteIDStartIndex,
                                  noteTextStartIndex - noteText.length() - noteIDStartIndex - 2);
    boardReq.noteText = body.substr(noteTextStartIndex, body.length() - noteTextStartIndex - 2);
    LOG_INFO << "BoardRequestHandler :: parseBoardRequest : crudType=[" << boardReq.crudType << "]\n";
    LOG_INFO << "BoardRequestHandler :: parseBoardRequest : boardID=[" << boardReq.boardID << "]\n";
    LOG_INFO << "BoardRequestHandler :: parseBoardRequest : boardPIN=[" << boardReq.boardPIN << "]\n";
    LOG_INFO << "BoardRequestHandler :: parseBoardRequest : noteID=[" << boardReq.noteID << "]\n";
    LOG_INFO << "BoardRequestHandler :: parseBoardRequest : noteText=[" << boardReq.noteText << "]\n";
    return boardReq;
}

bool BoardRequestHandler::isValidPIN(std::string PIN) {
    if (PIN.length() == 4 && isdigit(PIN[0]) && isdigit(PIN[1]) && isdigit(PIN[2]) && isdigit(PIN[3])) {
        return true;
    }
    return false;
}

bool BoardRequestHandler::isValidBoardID(std::string name) {
    if (name.length() < 1) {
        return false;
    }
    for (int i = 0; i < name.length(); i++) {
        if (!isalnum(name[i])) {
            return false;
	}
    }
    return true;
}

bool BoardRequestHandler::isValidNoteID(std::string ID) {
    if (ID.length() < 1) {
        return false;
    }
    for (int i = 0; i < ID.length(); i++) {
        if (!isdigit(ID[i])) {
            return false;
	}
    }
    return true;
}

