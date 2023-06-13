#ifndef ERRORREQUESTHANDLER_H
#define ERRORREQUESTHANDLER_H

#include "RequestHandler.h"

class ErrorRequestHandler : public RequestHandler {
    public:
        ErrorRequestHandler();
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif
