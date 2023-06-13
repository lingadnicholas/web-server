#include "MIME.h"

MIME :: MIME() {
}

std::string MIME:: getContentType (std::string fileExtension) {
    std::map<const std::string, std::string>::iterator it;
    it = content_type_.find(fileExtension);
    if (it == content_type_.end()) {
      return "application/octet-stream";
    }
    else {
      return it->second;
    }
}
