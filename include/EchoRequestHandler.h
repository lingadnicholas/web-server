#ifndef ECHOREQUESTHANDLER_H
#define ECHOREQUESTHANDLER_H

#include "RequestHandler.h"

class EchoRequestHandler : public RequestHandler {
    public:
        EchoRequestHandler();
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif
