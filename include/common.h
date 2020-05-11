#ifndef _COMMON_H_
#define _COMMON_H_
short min3(short a, short b, short c) {
    if(a < b) {
        return a < c ? a : c;
    } else {
        return b < c ? b : c;
    }
}
short absDiff(short a, short b) {
    if(a < b) {
        return b - a;
    } else {
        return a - b;
    }
}

#endif