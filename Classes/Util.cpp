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

int Util::getIndexByWeight(vector<int> &weight) {
    int count = 0;
    for (int i = 0; i < weight.size(); i++) {
        count += weight[i];
    }
    
    int index = 0;
    int random = rand()%count;
    count = 0;
    for (int i = 0; i < weight.size(); i++) {
        if (random >= count && random < (count + weight[i])) {
            index = i;
            break;
        }
        count += weight[i];
    }
    
    return index;
}