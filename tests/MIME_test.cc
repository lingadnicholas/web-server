#include "gtest/gtest.h"
#include "MIME.h"

class MimeTest : public ::testing::Test {
    protected:
        std::string fileExt; 
        MIME mime; 
};

TEST_F(MimeTest, txtExt) {
    fileExt = "txt"; 
    EXPECT_EQ("text/plain", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, htmExt) {
    fileExt = "htm"; 
    EXPECT_EQ("text/html", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, htmlExt) {
    fileExt = "html"; 
    EXPECT_EQ("text/html", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, jpgExt) {
    fileExt = "jpg"; 
    EXPECT_EQ("image/jpeg", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, jpegExt) {
    fileExt = "jpeg"; 
    EXPECT_EQ("image/jpeg", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, pngExt) {
    fileExt = "png"; 
    EXPECT_EQ("image/png", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, zipExt) {
    fileExt = "zip"; 
    EXPECT_EQ("application/zip", mime.getContentType(fileExt)); 
}

TEST_F(MimeTest, ExtNotFound) {
    fileExt = "ThisWillNeverBeAFileExtHopefully";
    EXPECT_EQ("application/octet-stream", mime.getContentType(fileExt)); 
}