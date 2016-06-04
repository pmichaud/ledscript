#ifndef TOPHAT_H
#define TOPHAT_H

#define SETUP() (paramd[p_foffset] = 2)
#define LED_NUM 60
#define KNOB_LED 2

#define LED_CODE
char code[CODE_NUM] =
    "+B/4C/4D/5E/5F/5G/5H/4I/4B !> " // rainbow chase
    "+:d50:%@ABCDEFGHI0!a;%a.34;(35:>;)"
    "+:d50:%@ABCDEFGHI0!%.35;(35:>;)"
    "+:%ABCDEFGHI!%=600;"            // rainbow fades
    "+:%ABCDEFGHI8,:d30!%60 "        // rainbow dots
    "+B "                            // red
    "+AB8 !> "                       // red chase
    "+G "                            // blue
    "+AG8 !> "                       // blue chase
    "+E "                            // green
    "+AE8 !> "                       // green chase
    "+C "                            // orange
    "+D "                            // yellow
    "+F "                            // aqua
    "+H "                            // purple
    "+I "                            // pink
    "+A "                            // white
    "+@ "                            // black
    "+:%aCCC0!%500;"                 // random white-on-gold pixels
    "+B6A6G6 !> "                    // red white blue chase
    "+:%ABAG8:q3,5:d30!%60 "         // red white blue white random
    ;

#endif
