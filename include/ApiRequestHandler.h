#ifndef APIREQUESTHANDLER_H
#define APIREQUESTHANDLER_H

#include "config_parser.h"
#include "constants.h"
#include <map>
#include "RequestHandler.h"
#include <string>
#include <vector>

class ApiRequestHandler : public RequestHandler {
    public:
        ApiRequestHandler(const std::string& path, NginxConfig* config, 
                          std::map<std::string, std::vector<int>>& entity_ids);
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);

    private:
        // location as specified in config (the matched portion of requestURI)
        std::string loc;

        // root as specified in config (the directory where the file should be looked for, to replace the location portion of the requestURI)
        std::string root; 

        // whether or not the config passed to this handler is bad (missing root /path; directive)
	    bool badConfig;

        // the map to hold the jsons (values) that exist at a certain path (key)
        std::map<std::string, std::vector<int>>& entity_ids;

        int getNextID(std::string directory);
        int handlePost(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory);
        int handleGet(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory);
        int handleList(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory);
        int handlePut(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory);
        int handleDelete(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory);
        int handleBadRequest(http::response<http::string_body>& res);
        int handleNotFound(http::response<http::string_body>& res);
};

#endif
