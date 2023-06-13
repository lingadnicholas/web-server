#ifndef STATICREQUESTHANDLER_H
#define STATICREQUESTHANDLER_H

#include "config_parser.h"
#include "constants.h"
#include <map>
#include <string>
#include "RequestHandler.h"

class StaticRequestHandler : public RequestHandler {
    public:
        StaticRequestHandler(const std::string& path, NginxConfig* config);
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);

    private:
        // location as specified in config (the matched portion of requestURI)
        std::string loc;

        // root as specified in config (the directory where the file should be looked for, to replace the location portion of the requestURI)
        std::string root; 

        // whether or not the config passed to this handler is bad (missing root /path; directive)
        bool badConfig;

        int handleFileNotFound(http::response<http::string_body>& res);
        int handleInternalServerError(http::response<http::string_body>& res);
};

#endif
