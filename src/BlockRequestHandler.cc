// include the header for your handler, RequestHandler
#include "BlockRequestHandler.h"

BlockRequestHandler :: BlockRequestHandler()
   : RequestHandler() {
   // log lets us know when a handler is being used
   LOG_INFO << "BlockRequestHandler :: Constructor\n";
}

int BlockRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    LOG_INFO << "BlockRequestHandler :: handleRequest\n";
    int block_time = 1; 
    std::string requestURI(req.target().begin(), req.target().end());
    try {
        block_time = std::stoi(requestURI.substr(requestURI.find_last_of('/')+1));
    } catch (const std::exception&) {
        LOG_INFO << "BlockRequestHandler :: handleRequest : Sleep time not specified or invalid format. Using default of 1.\n";
    }
    LOG_INFO << "BlockRequestHandler :: handleRequest : Sleep time: " << block_time << std::endl; 
    sleep(block_time); 

    std::string response_msg = "200 OK. Sleep " + std::to_string(block_time) + "s\r\n";
    res.version(req.version());
    res.result(HTTP_STATUS_OK);
    res.reason(HTTP_REASON_OK);
    res.set(http::field::content_type, CONTENT_TYPE_TEXT_PLAIN);
    res.set(http::field::content_length, std::to_string(response_msg.size()));
    res.body() = response_msg;

    return HTTP_STATUS_OK;
}
