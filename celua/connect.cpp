#include "connect.h"
#include "../tool/md5.h"
#include "../tool/base64.h"
#include "../tool/HttpConnect.h"


int time_enc(int _t) {
    return _t;
}

int time_dec(int enc) {
    return enc;
}


std::string encode(const std::string& text) {
    return text;
}

std::string decode(const std::string& enc) {
    return enc;
}

std::string encPost(const std::string& url, const std::string text[], size_t size) {
    return url;
}