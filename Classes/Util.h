//
//  Util.h
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#ifndef __IGGameC__Util__
#define __IGGameC__Util__

#include "cocos2d.h"

using namespace std;
using namespace cocos2d;

class Util {
public:
    static string strFormat(const char *format, ...);
    static void fitToSize(Node * node, Size size, bool border);
};

#endif /* defined(__IGGameC__Util__) */
