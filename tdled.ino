#include "FastLED.h"
#include <ctype.h>

#define NUM_LEDS 250
#define DATA_PIN 4

CRGB palette[64];
CRGB leds[NUM_LEDS];
int nled = 0;
int nfill = 0;

#define NUM_CODE 256
char code[NUM_CODE] = "?p9 !< ";
int pc = 0;
int pcstart = 0;
int framedelay = 50;


void setup() {
  delay(2000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pal64();
  Serial.begin(9600);
  Serial.setTimeout(100);
}

void loop() {
  nled = 0;
  pc = pcstart;

  while (pc < NUM_CODE && code[pc]) {
    int c = code[pc];
    switch (c) {
      case '!':
        pc++;
        pcstart = pc;
        break;
      case ':':
        pc++;
        coloncmd();
        break;
      case '<': 
        {
          int n = nfill;
          if (nled + n > NUM_LEDS) n = NUM_LEDS - nled;
          CRGB t = leds[nled];
          for (int i = 0; i < n - 1; i++) leds[nled+i] = leds[nled+i+1];
          leds[nled+n-1] = t;
          nled += n;
          pc++;
        }
        break;
      case '>':
        {
          int n = nfill;
          if (nled + n > NUM_LEDS) n = NUM_LEDS - nled;
          CRGB t = leds[nled + n - 1];
          for (int i = n-1; i > 0; i--) leds[nled+i] = leds[nled+i-1];
          leds[nled] = t;
          nled += n;
          pc++;
        }
        break;
      case ' ':
      case '\n':
        if (nled == 0) {
          pc++;
          continue;
        }
        nfill = nled;
        for (int j = nfill; j < NUM_LEDS; j++) {
          leds[j] = leds[j - nfill];
        }
        nled = NUM_LEDS;
      case ';':
        FastLED.show();
        delay(framedelay);
        nled = 0;
        pc++;
        if (Serial.available()) {
          readcode();
          return;
        }
        break;
      default:
        if (c >= 0x3f && c <= 0x7f) {
          CRGB color = palette[c & 0x3f];
          int n = scanint(pc+1, 1);
          while (n > 0 && nled < NUM_LEDS) {
            leds[nled++] = color;
            n--;
          }
        }
        break;
    }
  }
}


int scanint(int sc, int val) {
  if (isdigit(code[sc])) {
    val = 0;
    while (isdigit(code[sc])) {
      val = val * 10 + (code[sc] - '0');
      sc++;
    }
  }
  pc = sc;
  return val;
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
}


void coloncmd() {
  int c = code[pc];
  switch (c) {
    case 'd':
      framedelay = scanint(pc + 1, 100);
      break;
  }
}

