#ifndef HEALTHREQUESTHANDLER_H
#define HEALTHREQUESTHANDLER_H

#include "RequestHandler.h"

class HealthRequestHandler : public RequestHandler {
    public:
        HealthRequestHandler();
        int handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res);
};

#endif
