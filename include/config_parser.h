#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

// An nginx config file parser.

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
    public:
        std::vector<std::string> tokens_;
        std::unique_ptr<NginxConfig> child_block_;
        std::string ToString(int depth);
};

// The parsed representation of the entire config.
class NginxConfig {
    public:
        std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
        std::string ToString(int depth = 0);
        int getListeningPortHelper();
        int getListeningPort();
        void getFilesystemPathsHelper(std::map<std::string, std::string> &fsPaths);
        std::map<std::string, std::string> getFilesystemPaths();
        std::string removeTrailingSlash(std::string path);

        /* populateHdlrMap extracts (from the NginxConfig) mappings
         * from Request URL path (as specified by location directive) to a pair
         * containing the handlerType (string, e.g. "StaticHandler") and the NginxConfig
         * for that handler, containing configuration info (e.g., root ./data).
         * It populates the map received as an argument.
         */
        void populateHdlrMap(std::map<std::string, std::pair<std::string, NginxConfig *>> &hdlrMap);
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
    public:
        NginxConfigParser() {}

        // Take a opened config file or file name (respectively) and store the
        // parsed config in the provided NginxConfig out-param.  Returns true
        // iff the input config file is valid.
        bool Parse(std::istream* config_file, NginxConfig* config);
        bool Parse(const char* file_name, NginxConfig* config);

    private:
        enum TokenType {
            TOKEN_TYPE_START = 0,
            TOKEN_TYPE_NORMAL = 1,
            TOKEN_TYPE_START_BLOCK = 2,
            TOKEN_TYPE_END_BLOCK = 3,
            TOKEN_TYPE_COMMENT = 4,
            TOKEN_TYPE_STATEMENT_END = 5,
            TOKEN_TYPE_EOF = 6,
            TOKEN_TYPE_ERROR = 7,
            TOKEN_TYPE_QUOTED_STRING = 8,
            TOKEN_TYPE_WHITESPACE = 9
        };

        enum TokenParserState {
            TOKEN_STATE_INITIAL_WHITESPACE = 0,
            TOKEN_STATE_SINGLE_QUOTE = 1,
            TOKEN_STATE_DOUBLE_QUOTE = 2,
            TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
            TOKEN_STATE_TOKEN_TYPE_NORMAL = 4,
            TOKEN_STATE_WHITESPACE = 5
        };

        const char* TokenTypeAsString(TokenType type);
        TokenType ParseToken(std::istream* input, std::string* value);
};

#endif
