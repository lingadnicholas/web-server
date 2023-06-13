#include "HealthRequestHandler.h"

HealthRequestHandler :: HealthRequestHandler() 
    : RequestHandler() {
    LOG_INFO << "HealthRequestHandler :: Constructor\n";
}

int HealthRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    std::string ok_msg = "OK\r\n";
    
    res.version(req.version());
    res.result(HTTP_STATUS_OK);
    res.reason(HTTP_REASON_OK);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
    res.set(http::field::content_length, std::to_string(ok_msg.size()));
    res.body() = ok_msg;

    return HTTP_STATUS_OK;
}
