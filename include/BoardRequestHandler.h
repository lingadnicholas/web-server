#ifndef BOARDREQUESTHANDLER_H
#define BOARDREQUESTHANDLER_H

#include "config_parser.h"
#include "constants.h"
#include <map>
#include "RequestHandler.h"
#include <string>
#include <vector>

struct BoardRequest {
    std::string crudType;
    std::string boardID;
    std::string boardPIN;
    std::string noteID;
    std::string noteText;
};

class BoardRequestHandler : public RequestHandler {
    public:
        BoardRequestHandler(const std::string& path, NginxConfig* config, 
                          std::map<std::string, std::vector<int>>& board_notes, std::map<std::string, int>& board_pins);
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);

    private:
        // location as specified in config (the matched portion of requestURI)
        std::string loc;

        // root as specified in config (the directory where the file should be looked for, to replace the location portion of the requestURI)
        std::string root; 

        // whether or not the config passed to this handler is bad (missing root /path; directive)
	    bool badConfig;

        // the map to hold the jsons (values) that exist at a certain path (key)
        std::map<std::string, std::vector<int>>& board_notes;

        // the map to hold the pins (values) that exist for a certain path (board)
        std::map<std::string, int>& board_pins;

        int createBoard(const BoardRequest req, http::response<http::string_body>& res);
        int viewBoard(const BoardRequest req, http::response<http::string_body>& res);
        int getBoards(const BoardRequest req, http::response<http::string_body>& res);
        int createNote(const BoardRequest req, http::response<http::string_body>& res);
        int deleteNote(const BoardRequest req, http::response<http::string_body>& res);
        int updateNote(const BoardRequest req, http::response<http::string_body>& res);

        bool boardExists(std::string directory);
        bool noteExists(std::string boardID, std::string noteID);	
        bool atMaxNotes(std::string directory); 
        bool validatePIN(std::string directory, int PIN); 
        int getNextID(std::string directory);
        int handleBadRequest(http::response<http::string_body>& res, std::string reason);
        int handleInternalServerError(http::response<http::string_body>& res, std::string reason);
        int handleNotFound(http::response<http::string_body>& res);
        BoardRequest parseBoardRequest(std::string body);
        bool isValidPIN(std::string PIN);
        bool isValidBoardID(std::string name);
        bool isValidNoteID(std::string ID);
};

#endif
