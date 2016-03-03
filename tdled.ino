#include "FastLED.h"

#define NUM_LEDS 250
#define DATA_PIN 4

CRGB leds[NUM_LEDS];
CRGB palette[64];

void setup() {
    delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    pal64();
    Serial.begin(9600);
}


// char code[] = "P>\nD>\nA>\n";
char code[] = "?ppppppppp> p?pppppppp> pp?ppppppp> ppp?pppppp> pppp?ppppp> ppppp?pppp> pppppp?ppp> ppppppp?pp> pppppppp?p> ppppppppp?> ";
//  char code[] = "UPPPPPPPPP>\nPUPPPPPPPP>\nPPUPPPPPPP>\nPPPUPPPPPP>\nPPPPUPPPPP>\nPPPPPUPPPP>\nPPPPPPUPPP>\nPPPPPPPUPP>\nPPPPPPPPUP>\nPPPPPPPPPU>\n";
//  char code[] = "A>\nA>\nA>\nB>\nB>\nB>\nC>\nC>\nC>\nB>\nB>\nB>\n";


void loop() {
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
      case ' ':
        FastLED.show();
        delay(10);
        n = 0;
        break;
      default:
        if (code[i] >= 0x3f && code[i] <= 0x7f) 
          leds[n++] = palette[code[i] & 0x3f];
        break;
    }
  }
  FastLED.show();
}


void pal64() {
  // initialize palette with 6-bit color palette
  int v[4] = { 0x00, 0x55, 0xaa, 0xff };
  int n = 0;
  for (int r = 0; r < 4; r++) {
    for (int g = 0; g < 4; g++) {
      for (int b = 0; b < 4; b++) {
        palette[n++] = CRGB(v[r], v[g], v[b]);
      }
    }
  }
}

