#ifndef I7172_H
#define I7172_H

#include "FastLED.h"

#define LED_ORDER GRB
#define LED_NUM 150
#define CODE_NUM 256

#define LED_CODE
char code[CODE_NUM] = 
    "+B/6C/6D/6E/6F/6GG/6H/6I/6B !> " // rainbow chase
    "+:%ABCDEFGHI!%=600;"            // rainbow fades
    "+:%ABCDEFGHI8:d30!%60 "         // rainbow dots
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
    "+:%ACCC0!%500;"                 // random white-on-gold pixels
    "+B6A6G6 !> "                    // red white blue chase
    "+:%ABBBAGGG:q7,9!%60 "              // red white blue white random
    ;

#endif
