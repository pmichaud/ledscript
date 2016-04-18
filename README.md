TDled is a sketch to allow dynamic control of WS2812B LED arrays.  

The sketch reads LED scripts from the serial port and stores 
them in the code[] array.  Each character of the script represents
either a LED pixel value or some command to be performed.

TDled has a palette of 64 LED colors available.  In a script, byte
values of 0x40 through 0x7f are used to select a color from the
palette -- these byte values correspond to the ASCII characters '@'
through DEL.  Since DEL is often difficult to encode in text, the
byte value 0x3f ('?') is treated as a synonym for 0x7f.

The default palette is initialized with colors having 4 levels of
red (off/low/med/full), 4 levels of blue, and 4 levels of green.
Thus the first 32 entries of the default palette are:

```
   ASCII   R   G   B       Color           ASCII   R   G   B       Color
     @     00  00  00      Black             P     22  00  00      Red 33%
     A     00  00  22      Blue 33%          Q     22  00  22      Magenta 33%
     B     00  00  77      Blue 66%          R     22  00  77
     C     00  00  FF      Blue 100%         S     22  00  FF
     D     00  22  00      Green 33%         T     22  22  00      Yellow 33%
     E     00  22  22      Cyan 33%          U     22  22  22      White 33%
     F     00  22  77                        V     22  22  77
     G     00  22  FF                        W     22  22  FF
     H     00  77  00      Green 66%         X     22  77  00
     I     00  77  22                        Y     22  77  22
     J     00  77  77      Cyan 66%          Z     22  77  77
     K     00  77  FF                        [     22  77  FF
     L     00  FF  00      Green 100%        \     22  FF  00
     M     00  FF  22                        ]     22  FF  22
     N     00  FF  77                        ^     22  FF  77
     O     00  FF  FF      Cyan 100%         _     22  FF  FF
```

Other notable entries in the default palette:

```
   ASCII   R   G   B       Color           
     `     77  00  00      Red 66%
     b     77  00  77      Magenta 66%
     h     77  77  00      Yellow 66%
     j     77  77  77      White 66%
     p     FF  00  00      Red 100%
     s     FF  00  FF      Magenta 100%
     |     FF  FF  00      Yellow 100%
     ?     FF  FF  FF      White 100%
```

Thus one can specify the pixels of the LED strip by simply stringing together
a sequence of pixel colors:

     AAA@DD@PPPPP     # 3 blue, 1 black, 2 green, 1 black, 5 red

A pixel character can be followed by a number to indicate it should be 
repeated that many times.  Thus the above line could also be specified as:

     A3@D2@P5         # 3 blue, 1 black, 2 green, 1 black, 5 red

If terminated by a space or a newline, a pixel sequence will
be used to fill to the end of the LED strip.  To prevent the fill,
end the sequence with a semicolon.

     A3@D2@P5         # fill strip with this sequence
     A3@D2@P5;        # fill only first 12 LEDs

A strip can animate through a sequence of patterns by separating each
pattern with a space or newline.  Thus a simple 10-frame chase sequence
(white pixels chasing through a red background) can be given as:

    ?P9 P?P8 P2?P7 P3?P6 P4?P5 P5?P4 P6?P3 P7?P2 P8?P P9?

Note how the white pixel ('?') is stepped through the background of red
pixels ('P').

When the sketch is running, it monitors the serial port for a new
LED script to run.  Once a new LED script is loaded, the sketch immediately
switches to displaying the new script.  In Linux, this means the LED
strip can be quickly changed from the command line:

    echo "@" >/dev/ttyACM0         # turn all pixels off
    echo "A" >/dev/ttyACM0         # turn all pixels to 33% blue
    echo "PDA@" >/dev/ttyACM0      # sequence red-green-blue-black
    echo "P D A @" >/dev/ttyACM0   # all red, all green, all blue, all black

A few example scripts are given in the scripts/ directory.

Other character commands are available:

  / <n> <color> - ramp to color
    This command ramps <n> pixels from the color of the last pixel set to
    the target <color>.  Thus the sequence "@/15p" draws a black pixel (@)
    then sets the next 15 pixels to ramp from black to full red (P).
    The sequence can be chained, so a typical rainbow pattern might be
        'P/8T/8D/8E/8A/8Q/7P '   - red->yellow->green->cyan->blue->magenta->red

    (Speculation:  the '//' command might draw only to the (n-1)th pixel
     of the ramp, to allow seamless circular chains.)

  < - rotate left
  > - rotate right
    This command rotates a sequence of pixels left or right.  The rotation
    moves all of the pixels in the sequence one unit left or right, feeding
    the first pixel moved around to the other end.  By default, the number
    of pixels in the rotation sequence is given by the length of the last
    fill pattern from a previous ' ' or '\n' frame command.  (Speculation:
    future versions of this command will take an optional rotation length.)

    Thus a LEDscript like
         'P9? > > > > > > > > > '
    creates a frame of ten repeated pixels (9 red and one white), then
    nine successive frames by rotating the first ten pixels one space to
    the right and repeating that new sequence.  The net effect is a "chase"
    pattern.

  ! - restart here
    This command marks a new "restart point" when the end of a LEDscript is
    reached; instead of looping back to the beginning of the LEDscript, it
    loops back to the exclamation point.  It's useful for performing one-time
    initialization at the start of the LEDscript.
        'P9? !> '
    This creates a frame with a repeated pattern of 9 red and one white 
    pixel, then repeats (forever) frames consisting of rotate right 
    operations.


