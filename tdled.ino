#include "FastLED.h"

#define NUM_LEDS 250
#define DATA_PIN 4

CRGB leds[NUM_LEDS];

CRGB palette[64];

void setup() {
    delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    int r,g,b;
    int n = 0;
    for (r = 0; r < 0x100; r += 0x55) {
      for (g = 0; g < 0x100; g += 0x55) {
        for (b = 0; b < 0x100; b += 0x55) {
          palette[n++] = CRGB(r,g,b);
        }
      }
    }
    Serial.begin(9600);
}


char code[] = "P>\nD>\nA>\n";

void loop() {

//  char code[] = "?ppppppppp>\np?pppppppp>\npp?ppppppp>\nppp?pppppp>\npppp?ppppp>\nppppp?pppp>\npppppp?ppp>\nppppppp?pp>\npppppppp?p>\nppppppppp?>\n";
//  char code[] = "UPPPPPPPPP>\nPUPPPPPPPP>\nPPUPPPPPPP>\nPPPUPPPPPP>\nPPPPUPPPPP>\nPPPPPUPPPP>\nPPPPPPUPPP>\nPPPPPPPUPP>\nPPPPPPPPUP>\nPPPPPPPPPU>\n";
//  char code[] = "A>\nA>\nA>\nB>\nB>\nB>\nC>\nC>\nC>\nB>\nB>\nB>\n";
  int n = 0;
  int j;
  Serial.println("Hello world\n");
  while (Serial.available()) {
    int c = Serial.read();
    if (c >= '?') code[0] = c;
  }
  for(int i = 0; code[i]; i++) {
    switch (code[i]) {
      case '>':
        for (j=n; j<NUM_LEDS; j++) leds[j] = leds[j-n];
        n = j;
        break;
      case '\n':
        FastLED.show();
        delay(1000);
        n = 0;
        break;
      default:
        leds[n++] = palette[code[i] & 0x3f];
        break;
    }
  }
  FastLED.show();
}

