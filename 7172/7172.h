#ifndef I7172_H
#define I7172_H

#include "FastLED.h"

#define LED_ORDER RGB
#define LED_NUM 42

#define LED_CODE
#define CODE_NUM 256
char code[CODE_NUM] = 
    "p/8s/8C/8O/8L/8|/8p !> "     // rainbow chase
    "+p "
    "+?p6 !> "                    // red chase
    "+C "
    "+?C6 !> "                    // blue chasee 
    "+L "
    "+?L6 !> "                    // green chase
    "+s "                         // magenta
    "+O "                         // cyan
    "+| "                         // yellow
    "+? "                         // white
    "+@ "                         // black
    // "+p7?7C7 "                    // red white blue
    // "+p7?7C7 !> "                 // red white blue chase
    "+:d100:%xxx?!%500;"               // random white-on-gold pixels
    "+:d800p42;:d200?10p32;p10?10p22;p20?10p12;p30?12;"  // 7-1-7-2 blue
    "+:d800C42;:d200?10C32;C10?10C22;C20?10C12;C30?12;"  // 7-1-7-2 blue
    ;

#endif
