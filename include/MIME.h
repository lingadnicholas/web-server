#ifndef MIME_H
#define MIME_H

#include <map>
#include <string>

class MIME {
    public: 
        MIME(); 
        std::string getContentType(std::string fileExtension); 

    private: 
        std::map<const std::string, std::string> content_type_ = {
            {"txt", "text/plain"},
            {"htm", "text/html"}, 
            {"html", "text/html"},
            {"jpg", "image/jpeg"}, 
            {"jpeg", "image/jpeg"},
            {"png", "image/png"},
            {"zip", "application/zip"}
        };
};

#endif
