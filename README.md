LEDscript is a sketch to allow dynamic control of WS2812B LED arrays
with Arduino-based controllers.

The sketch stores "LED scripts" in a character array (these can be
loaded dynamically via the serial port) and interprets the scripts
to set the colors of the LED array.  The scripting language is highly
compact to reduce overall storage requirements.  Indeed, the entire
sketch is designed to make it possible to have good control of a
300-element LED strip using an Arduino Uno type controller (with
only 2K of SRAM).  Larger LED strips or highly complex LED sequences
may require using an Arduino Mega or device with more SRAM available.
upgrade to an Arduino Mega.

Each character of a script represents either the color of an LED to
be set or some command to be performed.  LEDscript starts with a 
palette of 64 colors, identified by values 0x40 through 0x7f -- these
correspond to the ASCII characters '@' through DEL.  Since DEL is
often difficult to encode in a text string, the character '?' (0x3f)
is a synonym for DEL.

The default palette is initialized with black, white, various 
brightnesses of rainbow-flavored hues, and a couple of "grays":

                        Medium        Dark       Light    
  Black    @ 000000                            ` 0b0b0b
  White    A ababab                            a ffffff
  Red      B ff0000    J 410000    R 0b0000    b c12828
  Orange   C ab5500    K 2c1600    S 080400    c 8f5b28
  Yellow   D abab00    L 2c2c00    T 080800    d 8f8f28
  Green    E 00ff00    M 004100    U 000b00    e 28c128
  Aqua     F 00ab55    N 002c16    V 000804    f 288f5b
  Blue     G 0000ff    O 000041    W 00000b    g 2828c1   
  Purple   H 5500ab    P 16002c    X 040008    h 5b288f
  Pink     I ab0055    Q 2c0016    Y 080004    i 8f285b

  Bright Orange:  [ ff8000
  Bright Yellow:  \ ffff00
  Bright Cyan:    ] 00ffff
  Bright Purple:  ^ 8000ff
  Bright Magenta: _ ff00ff

The first column are standard rainbow colors with full saturation
and relatively stable luminosity; the "Medium" and "Dark" columns
have the same hue/saturation but are less luminous.  The "Light"
column has the colors slightly desaturated (so they appear
lighter, more pastel-ish).

The "Bright" versions are the same hues but with R-G-B components
turned up full; thus "Bright Yellow" (0xffff00) is likely to have
much more luminance than a full "Red" (0xff0000) would have, simply
because the yellow has light emitting from two diodes instead of one.

One can specify the pixels to be set in a LED strip by simply stringing
together letters corresponding to pixel colors:

    CCC@HH@BBBBB      # 3 orange, 1 black, 2 purple, 1 black, 5 red

A pixel character can be followed by a number to indicate it should be 
repeated that many times.  Thus the above line could also be specified as:

    C3@H2@B5          # 3 orange, 1 black, 2 purple, 1 black, 5 red

If a sequence contains a space or newline, then the most recent
sequence of pixels will be repeated to fill to the end of the LED
array.  To suppress any filling from taking place, end the sequence
with a semicolon.

    C3@H2@B5          # repeat sequence throughout LED strip
    C3@H2@B5;         # set only first 12 LEDs

A sequence ending with a space, newline, or semicolon is called 
a "frame".  Specifying a sequence of multiple frames (i.e., pixel
sequences separated by spaces, newlines, or semicolons) will cause
each frame to display in succession.  A simple 10-frame chase sequence
of white pixels chasing through a red background can be given as:

    AB9 BAB8 B2AB7 B3AB6 B4AB5 B5AB4 B6AB3 B7AB2 B8AB B9A

Note how the white pixel ('A') is successively stepped through a
group of red pixels ('B') in each frame.

The delay between frames (thus the frame rate) can be set using ":d"
followed by the number of milliseconds between frames... the default
is 100 milliseconds.

    # fast chase, delay = 5ms
    :d5AB9 BAB8 B2AB7 B3AB6 B4AB5 B5AB4 B6AB3 B7AB2 B8AB B9A
    # slow chase, delay = 1000ms
    :d1000AB9 BAB8 B2AB7 B3AB6 B4AB5 B5AB4 B6AB3 B7AB2 B8AB B9A

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

```
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
```
