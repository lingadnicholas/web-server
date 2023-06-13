#ifndef REQUESTHANDLERFACTORY_H
#define REQUESTHANDLERFACTORY_H

#include "ApiRequestHandler.h"
#include "BoardRequestHandler.h"
#include "BlockRequestHandler.h"
#include "config_parser.h"
#include "EchoRequestHandler.h"
#include "ErrorRequestHandler.h"
#include "HealthRequestHandler.h"
#include "log.h"
#include <map>
#include "RequestHandler.h"
#include "StaticRequestHandler.h"
#include <string>
#include <vector>

class RequestHandlerFactory {
    public:
        virtual RequestHandler* create(std::string loc, NginxConfig* cfg) = 0;
};

class EchoHandlerFactory : public RequestHandlerFactory {
    public:
        EchoHandlerFactory();
        EchoRequestHandler* create(std::string loc, NginxConfig* cfg);
};

class StaticHandlerFactory : public RequestHandlerFactory {
    public:
        StaticHandlerFactory();
        StaticRequestHandler* create(std::string loc, NginxConfig* cfg);
};

class ErrorHandlerFactory : public RequestHandlerFactory {
    public:
        ErrorHandlerFactory();
        ErrorRequestHandler* create(std::string loc, NginxConfig* cfg);
};

class ApiHandlerFactory : public RequestHandlerFactory {
    public:
        ApiHandlerFactory();
        ApiRequestHandler* create(std::string loc, NginxConfig* cfg);
    private:
        std::map<std::string, std::vector<int>> entity_ids;
};

class BlockHandlerFactory : public RequestHandlerFactory {
   public:
       BlockHandlerFactory();
       BlockRequestHandler* create(std::string loc, NginxConfig* cfg);
};

class HealthHandlerFactory : public RequestHandlerFactory {
   public:
       HealthHandlerFactory();
       HealthRequestHandler* create(std::string loc, NginxConfig* cfg);
};

class BoardHandlerFactory : public RequestHandlerFactory {
   public:
       BoardHandlerFactory();
       BoardRequestHandler* create(std::string loc, NginxConfig* cfg);
   private: 
        std::map<std::string, std::vector<int>> entity_ids;
        std::map<std::string, int> board_pins;

};
#endif
