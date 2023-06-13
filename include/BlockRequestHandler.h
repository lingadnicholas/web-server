#ifndef BLOCKREQUESTHANDLER_H
#define BLOCKREQUESTHANDLER_H

#include "RequestHandler.h"

class BlockRequestHandler : public RequestHandler {
    public:
        BlockRequestHandler();
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif
