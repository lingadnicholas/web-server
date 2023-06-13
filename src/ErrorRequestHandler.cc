#include "ErrorRequestHandler.h"

ErrorRequestHandler :: ErrorRequestHandler() 
    : RequestHandler() {
    LOG_INFO << "ErrorRequestHandler :: Constructor\n";
}

int ErrorRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    std::string error_msg = "404 Not Found\r\n";
    
    res.version(req.version());
    res.result(HTTP_STATUS_NOT_FOUND);
    res.reason(HTTP_REASON_NOT_FOUND);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
    res.set(http::field::content_length, std::to_string(error_msg.size()));
    res.body() = error_msg;

    return HTTP_STATUS_NOT_FOUND;
}
