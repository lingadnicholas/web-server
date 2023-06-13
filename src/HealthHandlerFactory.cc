#include "RequestHandlerFactory.h"

HealthHandlerFactory :: HealthHandlerFactory() {
    LOG_INFO << "HealthHandlerFactory :: Constructor\n";
};

HealthRequestHandler* HealthHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    return new HealthRequestHandler();
}
