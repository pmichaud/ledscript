#include "FastLED.h"
#include <ctype.h>

#define NUM_LEDS 150
#define DATA_PIN 4

CRGB palette[64];
CRGB leds[NUM_LEDS];
int nleds = 0;
int nfill = 0;

#define NUM_RPAL 64
char rpal[NUM_RPAL] = "@@@?";
int nrpal = 4;

#define NUM_CODE 256
char code[NUM_CODE] = "UP9 !< ";
int pc = 0;
int pcstart = 0;
int framedelay = 50;


void setup() {
  delay(500);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pal64();
  Serial.begin(9600);
  Serial.setTimeout(250);
}

void loop() {
  nleds = 0;
  pc = pcstart;

  while (pc < NUM_CODE && code[pc]) {
    int c = code[pc];
    if (Serial.available()) {
      readcode();
      return;
    }
    switch (c) {
      case '!':  // set new starting point
        pc++;
        pcstart = pc;
        break;
      case ':':  // special colon-based command
        pc++;
        coloncmd();
        break;
      case '%':  // pick colors from random palette
        {
          int n = scanint(pc+1, 1);
          while (n > 0 && nleds < NUM_LEDS) {
            leds[nleds++] = palette[rpal[random(nrpal)] & 0x3f];
            n--;
          }
        }
        break;
      case '/': if (nleds>0) rampTo(leds[nleds-1]); break;
      case '<': 
        {
          int n = nfill;
          if (nleds + n > NUM_LEDS) n = NUM_LEDS - nleds;
          CRGB t = leds[nleds];
          for (int i = 0; i < n - 1; i++) leds[nleds+i] = leds[nleds+i+1];
          leds[nleds+n-1] = t;
          nleds += n;
          pc++;
        }
        break;
      case '>':
        {
          int n = nfill;
          if (nleds + n > NUM_LEDS) n = NUM_LEDS - nleds;
          CRGB t = leds[nleds + n - 1];
          for (int i = n-1; i > 0; i--) leds[nleds+i] = leds[nleds+i-1];
          leds[nleds] = t;
          nleds += n;
          pc++;
        }
        break;
      case ' ':
      case '\n':
	// fill and display
        if (nleds != 0) {
          nfill = nleds;
          for (int j = nfill; j < NUM_LEDS; j++) {
            leds[j] = leds[j - nfill];
          }
        }
        // fall through
      case ';':
        // display without filling
        FastLED.show();
        delay(framedelay);
        nleds = 0;
        pc++;
        break;
      default:
        // fill a color
        if (c >= 0x3f && c <= 0x7f) {
          CRGB color = palette[c & 0x3f];
          int n = scanint(pc+1, 1);
          while (n > 0 && nleds < NUM_LEDS) {
            leds[nleds++] = color;
            n--;
          }
        }
        break;
    }
  }
}


void rampTo(CRGB from) {
  int n = scanint(pc+1, 1);
  CRGB to = (code[pc]) ? palette[code[pc++] & 0x3f] : CRGB::Black;
  int r0 = from.r;  int rd = to.r - r0;
  int g0 = from.g;  int gd = to.g - g0;
  int b0 = from.b;  int bd = to.b - b0;
  for (int i = 1; i <= n && nleds < NUM_LEDS; i++) {
    leds[nleds++] = CRGB(r0+(rd*i)/n, g0+(gd*i)/n, b0+(bd*i)/n); 
  }
}

int scanint(int sc, int val) {
  if (isdigit(code[sc])) {
    val = 0;
    while (code[sc] && isdigit(code[sc])) {
      val = val * 10 + (code[sc] - '0');
      sc++;
    }
  }
  pc = sc;
  return val;
}


void pal64() {
  // initialize palette with 6-bit color palette
  int v[4] = { 0x00, 0x22, 0x77, 0xff };
  int n = 0;
  for (int r = 0; r < 4; r++) {
    for (int g = 0; g < 4; g++) {
      for (int b = 0; b < 4; b++) {
        palette[n++] = CRGB(v[r], v[g], v[b]);
      }
    }
  }
}


void readcode() {
  // read a new code string from the serial port
  int n = Serial.readBytes(code, NUM_CODE - 1);
  code[n] = 0;
  pc = 0;
  pcstart = 0;
  framedelay = 100;
  while (Serial.available()) {
    Serial.read();
  }
//  Serial.print(code);
}


void coloncmd() {
  int brightness;
  int c = code[pc];
  switch (c) {
    case 'd':
      framedelay = scanint(pc+1, 100);
      break;
    case 'b':
      brightness = scanint(pc+1, 255);
      FastLED.setBrightness(brightness);
      break;
    case '%':
      nrpal = 0;
      for (pc++; code[pc] >= 0x3f && code[pc] <= 0x7f; pc++)
        if (nrpal < NUM_RPAL) rpal[nrpal++] = code[pc];
      break;
  }
}

