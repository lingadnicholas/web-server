#include "RequestHandlerFactory.h"

BoardHandlerFactory :: BoardHandlerFactory() {
    LOG_INFO << "BoardHandlerFactory :: Constructor\n";
};

BoardRequestHandler* BoardHandlerFactory::create(std::string loc, NginxConfig* cfg) {
    LOG_INFO << "BoardHandlerFactory :: create\n";
    return new BoardRequestHandler(loc, cfg, entity_ids, board_pins);
}
