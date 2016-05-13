#ifndef I7172_H
#define I7172_H

#include "FastLED.h"

#define LED_ORDER GRB
#define LED_NUM 150
#define CODE_NUM 256

#define LED_CODE
char code[CODE_NUM] = 
    "+B/6C/6D/6E/6F/6GG/6H/6I/6B !> " // rainbow chase
    "+:%ABCDEFGHI!%=150;"            // rainbow fades
    "+:%ABCDEFGHI8,:d30!%60 "         // rainbow dots
    "+B "                            // red
    "+AB9 !> "                       // red chase
    "+G "                            // blue
    "+AG9 !> "                       // blue chase
    "+E "                            // green
    "+AE9 !> "                       // green chase
    "+C "                            // orange
    "+D "                            // yellow
    "+F "                            // aqua
    "+H "                            // purple
    "+I "                            // pink
    "+A "                            // white
    "+@ "                            // black
    "+:%aCCC0!%150;"                 // random white-on-gold pixels
    "+B8A9G8 !> "                    // red white blue chase
    "+:%ABBBAGGG:q7,9!%60 "              // red white blue white random
    ;

// 150 == (2,3,6,10,15,25,50,75)
// 149 == (1,149)
// 148 == (2,4,37,74)
// 147 == (3,7,21,49)

#endif
