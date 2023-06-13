#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include "log.h"
#include "MIME.h"
#include "StaticRequestHandler.h"

StaticRequestHandler :: StaticRequestHandler(const std::string& path,
                                             NginxConfig* config)
    : RequestHandler() {
    LOG_INFO << "StaticRequestHandler :: Constructor\n";
    loc = path;
    // Parse config in search of the root directive (base dir corresponding to this handler's location)
    if (config->statements_.size() < 1) {
        LOG_ERROR << "StaticRequestHandler :: Constructor : loc=" << path
                  << " has bad config, no statements in block\n";
        badConfig = true;
        return;
    }
    NginxConfigStatement *stmt = config->statements_[0].get(); // assume first statement is root /path; statement
    if (stmt->tokens_.size() != 2 || stmt->tokens_[0] != "root") {
        LOG_ERROR << "StaticRequestHandler :: Constructor : loc=" << path
                  << " has bad config, missing or wrongly named root directive\n";
        badConfig = true;
        return;
    }
    root = stmt->tokens_[1];
    LOG_INFO << "StaticRequestHandler :: Constructor : config is good, loc=" << path << " and root=" << root << "\n";
    badConfig = false;
}

int StaticRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    res.version(req.version());
    // if request method was not valid, method_string gets set to "", so return bad request in this case
    if (req.method_string()=="") {
        res.result(HTTP_STATUS_BAD_REQUEST);
        res.reason(HTTP_REASON_BAD_REQUEST);
        return HTTP_STATUS_BAD_REQUEST;
    }
    if (badConfig) {
        return handleInternalServerError(res);
    }
    if (req.method() != http::verb::get) {
        LOG_INFO << "StaticRequestHandler :: handleRequest : echoing non-GET request\n";
        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);
        res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);

        std::string body;
        std::string const strHeaders = boost::lexical_cast<std::string>(req.base());
        body += strHeaders;
        body += req.body();
        res.body() = body;
        res.set(http::field::content_length, std::to_string(res.body().size()));
        return HTTP_STATUS_OK;
    }

    std::string requestURI(req.target().begin(), req.target().end());
    std::string fileExtension = requestURI.substr(requestURI.find_last_of(".") + 1);
    std::string filePath = requestURI;
    filePath = root + requestURI.substr(loc.length()); 
    // the line above replaces the location portion of the requestURI with the root path
    // resulting in the filePath, that is, the path where the file is to be looked for

    LOG_INFO << "StaticRequestHandler: handleRequest : Longest matched path for " << requestURI << " is " << loc << "\n";
    LOG_INFO << "StaticRequestHandler: handleRequest : File path to be used is " << filePath << "\n";
    std::ifstream istream(filePath, std::ios::in | std::ios::binary);
    // if file is not found, not readable, or is a directory or other non-regular file type
    if (!istream.good() || !boost::filesystem::is_regular_file(filePath)) {
        LOG_INFO << "StaticRequestHandler :: handleRequest : returning 404 File Not Found\n";
        return handleFileNotFound(res);
    }
    else { // file found
        std::string body((std::istreambuf_iterator<char>(istream)),
                         (std::istreambuf_iterator<char>()));
        // Resource used for the two lines above:
        // https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
        int content_length = body.length();
        MIME mime; 
        std::string content_type = mime.getContentType(fileExtension); 

        res.set(http::field::content_type, content_type);
        res.set(http::field::content_length, std::to_string(content_length));

        res.result(HTTP_STATUS_OK);
        res.reason(HTTP_REASON_OK);

        res.body() = body;

        return HTTP_STATUS_OK;
    }
    // should not reach this
    res.result(HTTP_STATUS_BAD_REQUEST);
    res.reason(HTTP_REASON_BAD_REQUEST);
    return HTTP_STATUS_BAD_REQUEST;
}

// handleFileNotFound returns the file not found status code
// and fills in the fields in the response reference object passed to it
int StaticRequestHandler :: handleFileNotFound(http::response<http::string_body>& res) {
    std::string error_msg = "404 Not Found\r\n";
    res.result(HTTP_STATUS_NOT_FOUND);
    res.reason(HTTP_REASON_NOT_FOUND);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;
    return HTTP_STATUS_NOT_FOUND;
}


// handleInternalServerError returns the internal server error status code
// and fills in the fields in the response reference object passed to it
int StaticRequestHandler :: handleInternalServerError(http::response<http::string_body>& res) {
    std::string error_msg = "500 Internal Server Error\r\n";
    res.result(HTTP_STATUS_INTERNAL_SERVER_ERROR);
    res.reason(HTTP_REASON_INTERNAL_SERVER_ERROR);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

