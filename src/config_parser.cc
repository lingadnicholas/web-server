// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include "config_parser.h"
#include <cstdio>
#include <fstream>
#include "log.h"
#include <stack>

std::string NginxConfigStatement::ToString(int depth) {
    std::string serialized_statement;
    for (int i = 0; i < depth; ++i) {
        serialized_statement.append("  ");
    }
    for (unsigned int i = 0; i < tokens_.size(); ++i) {
        if (i != 0) {
            serialized_statement.append(" ");
        }
        serialized_statement.append(tokens_[i]);
    }
    if (child_block_.get() != nullptr) {
        serialized_statement.append(" {\n");
        serialized_statement.append(child_block_->ToString(depth + 1));
        for (int i = 0; i < depth; ++i) {
            serialized_statement.append("  ");
        }
        serialized_statement.append("}");
    }
    else {
        serialized_statement.append(";");
    }
    serialized_statement.append("\n");
    return serialized_statement;
}

std::string NginxConfig::ToString(int depth) {
    std::string serialized_config;
    for (const auto& statement : statements_) {
        serialized_config.append(statement->ToString(depth));
    }
    return serialized_config;
}

int NginxConfig::getListeningPortHelper() {
    int port = -1; // invalid port number
    for (int i = 0; i < statements_.size(); i++) {
        NginxConfigStatement *stmt = statements_[i].get();
        if (stmt->tokens_.size() == 2 && stmt->tokens_[0] == "port") {
            port = std::stoi(stmt->tokens_[1]);
            return port;
        }
        else if (stmt->child_block_.get() != nullptr) {
            port = (*(stmt->child_block_.get())).getListeningPortHelper();
            if (port != -1) {
                return port;
            }
        }
    }
    return -1;
}

int NginxConfig::getListeningPort() {
    int port = this->getListeningPortHelper();
    if (port >= 0 && port <= 65535) {
        return port;
    }
    else {
        LOG_INFO << "NginxConfig :: getListeningPort : Port number missing or invalid value (not in [0, 65535] range). Using default of 8080." << std::endl;
        return 8080;
    }
}

void NginxConfig::getFilesystemPathsHelper(std::map<std::string, std::string> &fsPaths) {
    for (int i = 0; i < statements_.size(); i++) {
        NginxConfigStatement *stmt = statements_[i].get();
        if (stmt->tokens_.size() == 2 && stmt->tokens_[0] == "location") {
            NginxConfigStatement *child = (stmt->child_block_.get())->statements_[0].get();
            if (child->tokens_.size() == 2 && child->tokens_[0] == "root") {
                fsPaths[stmt->tokens_[1]] = child->tokens_[1];
            }
        }
        else if (stmt->child_block_.get() != nullptr) {
            (*(stmt->child_block_.get())).getFilesystemPathsHelper(fsPaths);
        }
    }
}

std::map<std::string, std::string> NginxConfig::getFilesystemPaths() {
    std::map<std::string, std::string> filesystemPaths;
    getFilesystemPathsHelper(filesystemPaths);
    return filesystemPaths;
}

std::string NginxConfig::removeTrailingSlash(std::string path) {
    std::string result = "";
    int pos = path.length() - 1;
    while(pos > 0 && path[pos] == '/') {
        pos--;
    }
    result = path.substr(0, pos + 1);
    return result;
}

void NginxConfig::populateHdlrMap(std::map<std::string, std::pair<std::string, NginxConfig*>> &hdlrMap) {
    for (int i = 0; i < statements_.size(); i++) {
        NginxConfigStatement *stmt = statements_[i].get();
        if (stmt->tokens_.size() == 3 && stmt->tokens_[0] == "location") {
            hdlrMap[removeTrailingSlash(stmt->tokens_[1])] = {stmt->tokens_[2], (stmt->child_block_).get()};
        }
    }
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
    std::stack<NginxConfig*> config_stack;
    config_stack.push(config);
    TokenType last_token_type = TOKEN_TYPE_START;
    TokenType second_last_token_type = TOKEN_TYPE_START;
    TokenType token_type;
    int level = 0; // level of nestedness (block level) used to ensure matched curly braces
    while (true) {
        std::string token;
        token_type = ParseToken(config_file, &token);
        printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
        if (token_type == TOKEN_TYPE_ERROR) {
            break;
        }

        if (token_type == TOKEN_TYPE_COMMENT) {
            // Skip comments.
            continue;
        }

        if (token_type == TOKEN_TYPE_WHITESPACE) {
            if (last_token_type != TOKEN_TYPE_WHITESPACE) {
                second_last_token_type = last_token_type;
                last_token_type = token_type;
            }
            continue;
        }

        if (token_type == TOKEN_TYPE_START) {
            // Error.
            break;
        } else if (token_type == TOKEN_TYPE_NORMAL || token_type == TOKEN_TYPE_QUOTED_STRING) {
            if (last_token_type == TOKEN_TYPE_START ||
                last_token_type == TOKEN_TYPE_WHITESPACE ||
                last_token_type == TOKEN_TYPE_STATEMENT_END ||
                last_token_type == TOKEN_TYPE_START_BLOCK ||
                last_token_type == TOKEN_TYPE_END_BLOCK ||
                (last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_NORMAL) ||
                (last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_QUOTED_STRING)) {
                if (!(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_NORMAL) &&
                    !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_QUOTED_STRING)) {
                    config_stack.top()->statements_.emplace_back(
                    new NginxConfigStatement);
                }
                config_stack.top()->statements_.back().get()->tokens_.push_back(token);
            } else {
                // Error.
                break;
            }
        } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
            if (last_token_type != TOKEN_TYPE_NORMAL && last_token_type != TOKEN_TYPE_QUOTED_STRING &&
               !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_NORMAL) &&
               !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_QUOTED_STRING)) {
                // Error.
                break;
            }
        } else if (token_type == TOKEN_TYPE_START_BLOCK) {
            if (last_token_type != TOKEN_TYPE_NORMAL && last_token_type != TOKEN_TYPE_QUOTED_STRING &&
               !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_NORMAL) &&
               !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_QUOTED_STRING)) {
                   // Error.
                   break;
            }
            NginxConfig* const new_config = new NginxConfig;
            config_stack.top()->statements_.back().get()->child_block_.reset(new_config);
            config_stack.push(new_config);
            level++;
        } else if (token_type == TOKEN_TYPE_END_BLOCK) {
            // Allow empty directive block by allowing END BLOCK after START BLOCK token
            // Allow nested directive blocks by allowing END BLOCK after END BLOCK token
            if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
                last_token_type != TOKEN_TYPE_START_BLOCK &&
                last_token_type != TOKEN_TYPE_END_BLOCK &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_STATEMENT_END) &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_START_BLOCK) &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_END_BLOCK)) {
                    // Error.
                break;
            }
            if (level < 1) {
                // Error. No opening brace to correspond with this closing brace.
                std::cout << "Unmatched curly braces '}' -- Invalid config file syntax." << std::endl;
                break;
            }
            config_stack.pop();
            level--;
        } else if (token_type == TOKEN_TYPE_EOF) {
            // Allow empty file by allowing EOF after START token
            // If opening braces are left unmatched (level is not 0) then return false.
            if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
                last_token_type != TOKEN_TYPE_END_BLOCK &&
                last_token_type != TOKEN_TYPE_START &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_STATEMENT_END) &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_END_BLOCK) &&
                !(last_token_type == TOKEN_TYPE_WHITESPACE && second_last_token_type == TOKEN_TYPE_START)) {
                // Error.
                break;
            }
            if (level != 0) {
                std::cout << "Unmatched curly braces '{' -- Invalid config file syntax." << std::endl;
                break;
            }
            return true;
        } else {
            // Error. Unknown token.
            break;
        }
        second_last_token_type = last_token_type;
        last_token_type = token_type;
    }

    printf ("Bad transition from %s to %s to %s\n",
            TokenTypeAsString(second_last_token_type),
            TokenTypeAsString(last_token_type),
            TokenTypeAsString(token_type));
    return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
    std::ifstream config_file;
    config_file.open(file_name);
    if (!config_file.good()) {
        printf ("Failed to open config file: %s\n", file_name);
        return false;
    }

    const bool return_value =
        Parse(dynamic_cast<std::istream*>(&config_file), config);
    config_file.close();
    return return_value;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
    switch (type) {
        case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
        case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
        case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
        case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
        case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
        case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
        case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
        case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
        case TOKEN_TYPE_QUOTED_STRING: return "TOKEN_TYPE_QUOTED_STRING";
        case TOKEN_TYPE_WHITESPACE:    return "TOKEN_TYPE_WHITESPACE";
        default:                       return "Unknown token type";
    }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
    TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
    while (input->good()) {
        const char c = input->get();
        if (!input->good()) {
            break;
        }
        switch (state) {
            case TOKEN_STATE_INITIAL_WHITESPACE:
                switch (c) {
                    case '{':
                        *value = c;
                        return TOKEN_TYPE_START_BLOCK;
                    case '}':
                        *value = c;
                        return TOKEN_TYPE_END_BLOCK;
                    case '#':
                        *value = c;
                        state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
                        continue;
                    case '"':
                        *value = c;
                        state = TOKEN_STATE_DOUBLE_QUOTE;
                        continue;
                    case '\'':
                        *value = c;
                        state = TOKEN_STATE_SINGLE_QUOTE;
                        continue;
                    case ';':
                        *value = c;
                        return TOKEN_TYPE_STATEMENT_END;
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        state = TOKEN_STATE_WHITESPACE;
                        continue;
                    default:
                        *value += c;
                        state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
                        continue;
                }
            case TOKEN_STATE_SINGLE_QUOTE:
                // FIXED: the end of a quoted token should be followed by whitespace.
                // FIXED: define a QUOTED_STRING token type.
                // FIXED: Allow for backslash-escaping within strings.

                // check for backslash to allow escaping of \' \" \n \t and \\ in single quoted strings
                if (c == '\\') {
                    const char c_next = input->get();
                    if (c_next == '\'') {
                        *value += '\'';
                        continue;
                    }
                    else if (c_next == '"') {
                        *value += '"';
                        continue;
                    }
                    else if (c_next == 'n') {
                        *value += '\n';
                        continue;
                    }
                    else if (c_next == 't') {
                        *value += '\t';
                        continue;
                    }
                    else if (c_next == '\\') {
                        *value += '\\';
                        continue;
                    }
                    else {
                        input->unget();
                    }
                }
                *value += c;
                if (c == '\'') {
                    const char c_next = input->get();
                    // the next char after a quoted token must be a whitespace or semicolon or opening brace
                    if (c_next == ' ' || c_next == '\n' || c_next == '\r' || c_next == '\t' || c_next == ';' || c_next == '{') {
                        input->unget();
                        return TOKEN_TYPE_QUOTED_STRING;
                    }
                    std::cout << "ERROR: Quoted token must be followed by whitespace or semicolon" << std::endl;
                    return TOKEN_TYPE_ERROR;
                }
                continue;
            case TOKEN_STATE_DOUBLE_QUOTE:
                // check for backslash to allow escaping of \' \" \n \t and \\ in double quoted strings
                if (c == '\\') {
                    const char c_next = input->get();
                    if (c_next == '\'') {
                        *value += '\'';
                        continue;
                    }
                    else if (c_next == '"') {
                        *value += '"';
                        continue;
                    }
                    else if (c_next == 'n') {
                        *value += '\n';
                        continue;
                    }
                    else if (c_next == 't') {
                        *value += '\t';
                        continue;
                    }
                    else if (c_next == '\\') {
                        *value += '\\';
                        continue;
                    }
                    else {
                        input->unget();
                    }
                }
                *value += c;
                if (c == '"') {
                    const char c_next = input->get();
                    // the next char after a quoted token must be a whitespace or semicolon or opening brace
                    if (c_next == ' ' || c_next == '\n' || c_next == '\r' || c_next == '\t' || c_next == ';' || c_next == '{') {
                        input->unget();
                        return TOKEN_TYPE_QUOTED_STRING;
                    }
                    std::cout << "ERROR: Quoted token must be followed by whitespace or semicolon" << std::endl;
                    return TOKEN_TYPE_ERROR;
                }
                continue;
            case TOKEN_STATE_TOKEN_TYPE_COMMENT:
                if (c == '\n' || c == '\r') {
                    return TOKEN_TYPE_COMMENT;
                }
                *value += c;
                continue;
            case TOKEN_STATE_TOKEN_TYPE_NORMAL:
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
                    c == ';' || c == '{' || c == '}' || c == '\'' || c == '"') {
                    input->unget();
                    return TOKEN_TYPE_NORMAL;
                }
                *value += c;
                continue;
            case TOKEN_STATE_WHITESPACE:
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    *value += c;
                    continue;
                }
                else {
                    input->unget();
                    return TOKEN_TYPE_WHITESPACE;
                }
        }
    }

    // If we get here, we reached the end of the file.
    if (state == TOKEN_STATE_SINGLE_QUOTE ||
        state == TOKEN_STATE_DOUBLE_QUOTE) {
        return TOKEN_TYPE_ERROR;
    }

    return TOKEN_TYPE_EOF;
}