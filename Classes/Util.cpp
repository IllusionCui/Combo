//
//  Util.cpp
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#include "Util.h"

string Util::strFormat(const char *format, ...) {
    char buf[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, 4096, format, args);
    va_end(args);
    return string(buf);
}

void Util::fitToSize(Node * node, Size size, bool border) {
    float scaleX = size.width/node->getContentSize().width;
    float scaleY = size.height/node->getContentSize().height;
    node->setScale(border ? min(scaleX, scaleY) : max(scaleX, scaleY));
}