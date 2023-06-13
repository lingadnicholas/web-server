#include <algorithm>
#include "ApiRequestHandler.h"
#include <bits/stdc++.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include "log.h"
#include "MIME.h"

ApiRequestHandler :: ApiRequestHandler(const std::string& path,
                                             NginxConfig* config,
                                             std::map<std::string, std::vector<int>>& entity_ids)
    : RequestHandler(), loc(path), entity_ids(entity_ids) {
    LOG_INFO << "ApiRequestHandler :: Constructor\n";
    // parse config in search of root directive
    if (config == nullptr || config->statements_.size() < 1) {
        LOG_ERROR << "ApiRequestHandler :: Constructor : loc=" << path
                  << " has bad config, no statements in block\n";
        badConfig = true;
        return;
    }
    NginxConfigStatement *stmt = config->statements_[0].get(); // assume first statement is root /path; statement
    if (stmt->tokens_.size() != 2 || stmt->tokens_[0] != "root") {
        LOG_ERROR << "ApiRequestHandler :: Constructor : loc=" << path
                  << " has bad config, missing or wrongly named root directive\n";
        badConfig = true;
        return;
    }
    root = stmt->tokens_[1];
    LOG_INFO << "ApiRequestHandler :: Constructor : config is good, loc=" << path << " and root=" << root << "\n";
    badConfig = false;

    // Create root directory if it doesn't already exist
    if (!boost::filesystem::exists(root)) {
        LOG_INFO << "ApiRequestHandler :: Constructor : root path " << root << " does not exist, creating now\n";
        boost::filesystem::create_directories(root);
    }

    // load existing files from the root directory into the entity_ids map
    for (auto file : boost::filesystem::recursive_directory_iterator(root)) {
        if (!is_regular_file(file)) {
            LOG_INFO << "ApiRequestHandler :: Constructor : ignoring non-file in crud directory: " <<
                                        absolute(file).string() << std::endl;
            continue;
        }
        boost::filesystem::path file_path = absolute(file);
        std::string file_path_str = file_path.string();
        LOG_INFO << "ApiRequestHandler :: Constructor : file_path_str in root dir = " << file_path_str << "\n";
        int id = 0;
        // check that the filename is an integer
        if (isdigit(file_path_str[file_path_str.length()-1])) {
            id = std::stoi(file_path.stem().string());
        }
        else {
            LOG_INFO << "ApiRequestHandler :: Constructor : ignoring file with non-integer name";
            continue;
        }
        // remove the id and trailing slash part of the file path
        std::string entity = file_path_str.substr(0,file_path_str.find_last_of("/"));
        entity = entity.substr(entity.find_last_of("/")+1);
        std::vector<int> ids = entity_ids[entity];
        // Only insert if id is NOT in ids (otherwise we get duplicates due to short-lived handlers)
        if (std::find(ids.begin(), ids.end(), id) == ids.end()) {
            if (ids.size() == 0) {
                ids.push_back(id);
            }
            else if (ids.size() == 1) {
                if (ids[0] < id) {
                    ids.push_back(id);
                }
                else {
                    ids.insert(ids.begin(), id);
                }
            }
            else {
                int index = 1;
                while(index != ids.size() && (ids[index] == ids[index-1]+1)) {
                    index++;
                }
                ids.insert(ids.begin()+index,id);
            }
        }
        sort(ids.begin(), ids.end());
        entity_ids[entity] = ids;
        LOG_INFO << "ApiRequestHandler :: Constructor : set entity_ids entity = " << entity << " with ids:\n";
        for(int i = 0; i < ids.size(); i++) {
            LOG_INFO << "ApiRequestHandler :: Constructor : " << ids[i] << "\n";
        }
    }
}

int ApiRequestHandler :: handleRequest(http::request<http::string_body> req, http::response<http::string_body>& res) {
    if (badConfig) {
        std::string error_msg = "500 Internal Server Error\r\n";
        int contentLength = error_msg.length();
        std::string contentType = CONTENT_TYPE_TEXT_PLAIN;
        res.set(http::field::content_length, std::to_string(contentLength));
        res.set(http::field::content_type, contentType);
        res.result(HTTP_STATUS_INTERNAL_SERVER_ERROR);
        res.reason(HTTP_REASON_INTERNAL_SERVER_ERROR);
        res.body() = error_msg;
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    std::string requestURI = req.target().to_string();
    std::string directory;
    LOG_INFO << "ApiRequestHandler :: handleRequest : requestURI: " << requestURI << "\n";
    // target needs to be /loc/ followed by an entity
    if (!(requestURI.substr(0, loc.length()) == loc && requestURI.substr(loc.length(), 1) == "/") 
        || !(requestURI.length() > (loc.length() + 1)))  { // +1 for "/"
        LOG_ERROR << "ApiRequestHandler :: handleRequest : bad request uri";
        return handleBadRequest(res);
    }
    else {
        directory = requestURI.substr(loc.length() + 1); // +1 for "/"
    }
    http::verb method = req.method();
    LOG_INFO << "ApiRequestHandler :: handleRequest : handling " << 
            req.method_string() << " request with directory " << directory;

    switch (method) {
        case http::verb::post:
            //post requests can only have one entity and no id
            if (directory.find("/") != -1) {
                LOG_ERROR << "ApiRequestHandler :: handleRequest : invalid directory for post request";
                return handleBadRequest(res);
            }
            return handlePost(req, res, directory);
            break;
        case http::verb::get: // specific file was requested with GET request
            if (directory.find("/") != -1) { 
                return handleGet(req,res,directory);
            }
            else {
                return handleList(req,res,directory);
            }
            break;
        case http::verb::put:
            if (directory.find("/") == -1) {
                LOG_ERROR << "ApiRequestHandler :: handleRequest : invalid file for put request";
                return handleBadRequest(res);
            }
            return handlePut(req,res,directory);
            break; 
        case http::verb::delete_: 
            //delete requests must have 1 id 
            if (directory.find("/") == -1) {
                LOG_ERROR << "ApiRequestHandler :: handleRequest : invalid directory for delete request";
                return handleBadRequest(res);
            }
            return handleDelete(req,res,directory);
            break;
        default: // Bad request 
            LOG_ERROR << "ApiRequestHandler :: handleRequest : invalid method";
            return handleBadRequest(res);
            break;
    }
    return HTTP_STATUS_INTERNAL_SERVER_ERROR; 
}

int ApiRequestHandler :: getNextID(std::string directory) {
    std::vector<int> id_list = entity_ids[directory];
    int index = 1;
    int id = 0;
    if (id_list.size() == 0) {
        id = 1;
    }
    else if (id_list.size() == 1) {
        if (id_list[0] == 1) {
            id = 2;
        }
        else {
            id = 1;
        }
    }
    else {
        while(index != id_list.size() && (id_list[index] == id_list[index-1]+1)) {
            index++;
        }
        id = id_list[index-1]+1;
        index++;
    }
    id_list.insert(id_list.begin()+index-1,id);
    sort(id_list.begin(), id_list.end());
    entity_ids[directory] = id_list;
    return id;
}

int ApiRequestHandler::handlePost(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory) {
    std::string path = root + "/" + directory;
    boost::system::error_code ec;
    // create the specified path if it doesn't already exist in the file system
    if (!boost::filesystem::exists(path)) {
        boost::filesystem::create_directory(path, ec);
        if (ec) {
            LOG_ERROR << "ApiRequestHandler :: handlePost : error creating directory - " << ec;
            return handleBadRequest(res);
        }
    }
    int id = getNextID(directory);
    std::string file_path = path + "/" + std::to_string(id);
    std::string json = req.body();
    LOG_INFO << "ApiRequestHandler :: handlePost : creating file with path " << path;
    LOG_INFO << "ApiRequestHandler :: handlePost : creating file with json body [" << json << "]\n";
    std::ostringstream oss;
    oss << json;
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(file_path);
    file << body;
    file.close();

    std::string reply = "{\"id\": " + std::to_string(id) + "}";
    res.body() = reply;
    res.set(http::field::content_length, std::to_string(res.body().size()));
    res.set(http::field::content_type, CONTENT_TYPE_APPL_JSON);
    res.set(http::field::content_location, loc + "/" + directory + "/" + std::to_string(id));
    res.result(HTTP_STATUS_CREATED);
    res.reason(HTTP_REASON_CREATED);
    return HTTP_STATUS_CREATED;
}

int ApiRequestHandler::handleGet(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory) {
    std::string path = root + "/" + directory;
    boost::filesystem::path boost_path(path);
    //if the file doesn't exist, bad request 
    if (!boost::filesystem::exists(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handleGet : path does not exist: " << path << std::endl; 
        return handleNotFound(res);
    }
    if (boost::filesystem::is_directory(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handleGet : path is a directory: " << path << std::endl; 
        return handleBadRequest(res);
    }
    // get id 
    std::size_t found = directory.find_last_of("/");
    std::string id_str = directory.substr(found+1);
    std::string entity = directory.substr(0, found);
    int id; 
    try {
        id = std::stoi(id_str); 
    }
    catch(const std::exception&) {
        LOG_ERROR << "ApiRequestHandler :: handleGet : invalid id: " << id_str << std::endl; 
        return handleBadRequest(res); 
    }
    if (entity_ids.find(entity) != entity_ids.end()) {
        std::vector<int>::iterator vector_it; 
        vector_it = find(entity_ids[entity].begin(), entity_ids[entity].end(), id);
        if (vector_it == entity_ids[entity].end()) {
            LOG_ERROR << "ApiRequestHandler :: handleGet : entity: " << entity << "/ID: " << id << " not found in entity_ids\n";
            return handleNotFound(res);
        }
        std::ifstream file(path);
        std::stringstream body;
        body << file.rdbuf();
        file.close();

        // Write GET response
        res.result(HTTP_STATUS_OK);
        res.body() = body.str();
        res.content_length(res.body().size());
        res.set(http::field::content_type, "application/json");
        return HTTP_STATUS_OK;
    }
    return handleBadRequest(res);
}

int ApiRequestHandler::handleList(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory) {
    LOG_INFO << "ApiRequestHandler :: handleList\n";
    std::string path = root + "/" + directory;
    LOG_INFO << "ApiRequestHandler :: handleList : root = " << root << "\n";
    LOG_INFO << "ApiRequestHandler :: handleList : directory = " << directory << "\n";
    LOG_INFO << "ApiRequestHandler :: handleList : path = " << path << "\n";
    boost::filesystem::path boost_path(path);
    // if the directory doesn't exist, bad request 
    if (!boost::filesystem::is_directory(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handleList : path is not a directory: " << path << std::endl;
        return handleBadRequest(res);
    }
    std::string entity = directory;
    if (entity_ids.find(entity) != entity_ids.end()) {
        LOG_INFO << "ApiRequestHandler:: handleList : found entity = " << directory << " in entity_ids\n";
        // create list of IDs in entity_ids[entity]
        std::ostringstream oss;
        oss << "[";
        for(int i : entity_ids[entity]) {
            LOG_INFO << "ApiRequestHandler:: handleList : found file " << std::to_string(i) << " in entity_ids\n";
            oss << std::to_string(i) << ",";
        }
        oss.seekp(-1, std::ios_base::end); // this replaces the extra ',' with ']' at the end
        oss << "]";
        std::string body = oss.str();
        oss.clear();

        // Write LIST response
        res.result(HTTP_STATUS_OK);
        res.body() = body;
        res.content_length(res.body().size());
        res.set(http::field::content_type, "application/json");
        return HTTP_STATUS_OK;
    }
    LOG_ERROR << "ApiRequestHandler :: handleList : directory is empty: " << path << std::endl;
    return handleBadRequest(res);
}

int ApiRequestHandler::handlePut(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory) {
    std::string path = root + "/" + directory;
    boost::filesystem::path boost_path(path);
    // if the file doesn't exist, bad request 
    if (!boost::filesystem::exists(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handlePut : path does not exist: " << path << std::endl; 
        return handleBadRequest(res);
    }
    if (boost::filesystem::is_directory(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handlePut : is a directory: " << path << std::endl; 
        return handleBadRequest(res);
    }
    // get id 
    std::size_t found = directory.find_last_of("/");
    std::string id_str = directory.substr(found+1);
    std::string entity = directory.substr(0, found);
    int id; 
    try {
        id = std::stoi(id_str);
    }
    catch(const std::exception&) {
        LOG_ERROR << "ApiRequestHandler :: handlePut : invalid id: " << id_str << std::endl; 
        return handleBadRequest(res);
    }

    std::string json = req.body();
    LOG_INFO << "ApiRequestHandler :: handlePut : modifying file at path: " << path << std::endl;
    std::ostringstream oss;
    oss << json;
    std::string body = oss.str();
    oss.clear();
    std::ofstream file(path);
    file << body;
    file.close();
    // Write PUT response
    res.result(HTTP_STATUS_OK);
    res.body() = body;
    res.set(http::field::content_length, std::to_string(res.body().size()));
    res.set(http::field::content_type, "application/json");
    return HTTP_STATUS_OK;
}

int ApiRequestHandler::handleDelete(const http::request<http::string_body>& req, http::response<http::string_body>& res, std::string directory) {
    std::string path = root + "/" + directory;
    boost::filesystem::path boost_path(path);
    //if the file doesn't exist, bad request 
    if (!boost::filesystem::exists(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handleDelete : path does not exist: " << path << std::endl; 
        return handleBadRequest(res);
    }
    if (boost::filesystem::is_directory(boost_path)) {
        LOG_ERROR << "ApiRequestHandler :: handleDelete : path is a directory: " << path << std::endl; 
        return handleBadRequest(res);
    }
    // get id 
    std::size_t found = directory.find_last_of("/");
    std::string id_str = directory.substr(found+1);
    std::string entity = directory.substr(0, found);
    int id;
    try {
        id = std::stoi(id_str);
    }
    catch(const std::exception&) {
        LOG_ERROR << "ApiRequestHandler :: handleDelete : invalid id: " << id_str << std::endl; 
        return handleBadRequest(res); 
    }

    if (entity_ids.find(entity) != entity_ids.end()) {
        if (boost::filesystem::remove(boost_path)) {
            // Remove from entity_ids
            std::vector<int>::iterator vector_it; 
            vector_it = find(entity_ids[entity].begin(), entity_ids[entity].end(), id);
            if (vector_it == entity_ids[entity].end()) {
                LOG_ERROR << "ApiRequestHandler :: handleDelete : Entity: " << entity << "/ID: " << id << " not found in entity_ids\n";
                return handleBadRequest(res); 
            } 
            entity_ids[entity].erase(vector_it);
            LOG_INFO << "ApiRequestHandler :: handleDelete : Deleting file: " << directory << std::endl;
            res.result(HTTP_STATUS_OK);
            std::string body = "<html>"
                    "<head><title>Deleted</title></head>"
                    "<body><h1>File Deleted.</h1></body>"
                    "</html>";
            res.body() = body;
            res.set(http::field::content_length, std::to_string(res.body().size()));
            res.set(http::field::content_type, CONTENT_TYPE_TEXT_HTML);
            return HTTP_STATUS_OK;
        }
    }
    return handleBadRequest(res);
}

int ApiRequestHandler::handleBadRequest(http::response<http::string_body>& res) {
    std::string error_msg = "400 Bad Request\r\n";
    int contentLength = error_msg.length();
    std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
    res.set(http::field::content_length, std::to_string(contentLength));
    res.set(http::field::content_type, contentType);
    res.result(HTTP_STATUS_BAD_REQUEST);
    res.reason(HTTP_REASON_BAD_REQUEST);
    res.body() = error_msg;
    return HTTP_STATUS_BAD_REQUEST;
}

int ApiRequestHandler::handleNotFound(http::response<http::string_body>& res) {
    std::string error_msg = "404 Not Found\r\n";
    int contentLength = error_msg.length();
    std::string contentType = CONTENT_TYPE_TEXT_PLAIN; 
    res.set(http::field::content_length, std::to_string(contentLength));
    res.set(http::field::content_type, contentType);
    res.result(HTTP_STATUS_NOT_FOUND);
    res.reason(HTTP_REASON_NOT_FOUND);
    res.body() = error_msg;
    return HTTP_STATUS_NOT_FOUND;
}
