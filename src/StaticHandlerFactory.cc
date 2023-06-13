#include "RequestHandlerFactory.h"

StaticHandlerFactory :: StaticHandlerFactory() {
    LOG_INFO << "StaticHandlerFactory :: Constructor\n";
};

StaticRequestHandler* StaticHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    return new StaticRequestHandler(loc, cfg);
}
