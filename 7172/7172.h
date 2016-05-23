#ifndef I7172_H
#define I7172_H

#include "FastLED.h"

#define LED_ORDER RGB
#define LED_NUM 42 

#define LED_CODE
#define CODE_NUM 256
char code[CODE_NUM] = 
    "+:%ABCDEFGHI:d150!%=9%=9%=9%=11;"
    "+:%ABCDEFGHI8,:d30!%42;"
    "+B "                            // red
    "+aB6 !> "                       // red chase
    "+G "                            // blue
    "+aG6 !> "                       // blue chase
    "+E "                            // green
    "+aE6 !> "                       // green chase
    "+C "                            // orange
    "+D "                            // yellow
    "+F "                            // aqua
    "+H "                            // purple
    "+I "                            // pink
    "+A "                            // white
    "+@ "                            // black
    "+:%aCCC0!%50;"                 // random white-on-gold pixels
    //"+B6A6G6 !> "                    // red white blue chase
    //"+:%ABAG8:q3,5:d30!%60 "              // red white blue white random
    "+B42;;;;;;;;A10B32;;B10A10B22;;B20A10B12;;B30A12;;"  // 7-1-7-2 red
    "+G42;;;;;;;;A10G32;;G10A10G22;;G20A10G12;;G30A12;;"  // 7-1-7-2 red
    ;

#endif
