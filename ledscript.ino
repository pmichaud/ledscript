#include "FastLED.h"
#include <ctype.h>


#ifndef LED_PIN
#define LED_PIN 4
#endif
#ifndef LED_ORDER
#define LED_ORDER GRB
#endif


#ifndef LED_NUM
#define LED_NUM 60
#endif
CRGB ledv[LED_NUM];
int ledn = 0;
int ledfill = 0;

#define PALETTE_NUM 64
#define RPAL_NUM 64
CRGB palette[PALETTE_NUM];
char rpal[RPAL_NUM] = "@@@?";
int rpaln = 4;

#ifndef CODE_NUM
#define CODE_NUM 256
#endif

char code[CODE_NUM] = 
    "P6U !> "
    "+p/8s/8C/8O/8L/8|/8p !< "     // rainbow chase
    "+p "
    "+?p5 !< "                    // red chase
    "+C "
    "+?C5 !< "                    // blue chase
    "+L "
    "+?L5 !< "                    // green chase
    "+? "                         // white
    "+O "                         // cyan
    "+s "
    "+| "
    "+:%xxxx?!%149;"
    "+p7?7C7 "
    "+p7?7C7 !< "
    ;
int pc = 0;
int pcstart = 0;
int framedelay = 50;


void setup() {
  delay(500);
  Serial.begin(9600);
  Serial.setTimeout(250);
  FastLED.addLeds<WS2812B, LED_PIN, LED_ORDER>(ledv, LED_NUM);
  pal64();
}

void loop() {
  runcode();
}


int scanint(int sc, int val) {
  // scan an integer starting at <sc>, return <val> if no integer found
  // set <pc> at character following scanned integer
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


void runcode() {
  ledn = 0;
  pc = pcstart;
  while (pc < CODE_NUM && code[pc]) {
    int c = code[pc];
    switch (c) {
      case '!':  // set new starting point
        pc++;
        pcstart = pc;
        break;
      case '+':  // program boundary
        return;
      case '/': 
        pc++; rampTo(); break;
      case '<':
        pc++; rotateLeft(); break;
      case '>':
        pc++; rotateRight(); break;
      case ' ':
      case '\n':
        fillFrame();
        // fall through
      case ';':
        // display frame
        pc++;
        FastLED.show();
        delay(50);
        ledn = 0;
        break;
      default:
        // fill pixels with a color
        if (c >= 0x3f && c <= 0x7f) {
          CRGB color = palette[c & 0x3f];
          for (int n = scanint(pc+1, 1); n > 0 && ledn < LED_NUM; n--)
            ledv[ledn++] = color;
        }
        break;
    }
  }
}



void rampTo() {
  if (ledn > 0) {
    int n = scanint(pc+1, 1);
    CRGB from = ledv[ledn - 1];
    CRGB to = (code[pc]) ? palette[code[pc++] & 0x3f] : CRGB::Black;
    int r0 = from.r;  int rd = to.r - r0;
    int g0 = from.g;  int gd = to.g - g0;
    int b0 = from.b;  int bd = to.b - b0;
    for (int i = 1; i <= n && ledn < LED_NUM; i++) {
      ledv[ledn++] = CRGB(r0+(rd*i)/n, g0+(gd*i)/n, b0+(bd*i)/n); 
    }
  }
}


void fillFrame() {
  if (ledn != 0) {
    ledfill = ledn;
    for (int j = ledfill; j < LED_NUM; j++) {
      ledv[j] = ledv[j - ledfill];
    }
  }
}


void rotateLeft() {
  int n = ledfill;
  if (ledn + n > LED_NUM) n = LED_NUM - ledn;
  CRGB t = ledv[ledn];
  for (int i = 0; i < n - 1; i++) ledv[ledn+i] = ledv[ledn+i+1];
  ledv[ledn+n-1] = t;
  ledn += n;
}


void rotateRight() {
   int n = ledfill;
   if (ledn + n > LED_NUM) n = LED_NUM - ledn;
   CRGB t = ledv[ledn + n - 1];
   for (int i = n-1; i > 0; i--) ledv[ledn+i] = ledv[ledn+i-1];
   ledv[ledn] = t;
   ledn += n;
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


/* void readcode() {
  // read a new code string from the serial port
  int n = Serial.readBytes(code, CODE_NUM - 1);
  code[n] = 0;
  pc = 0;
  pcstart = 0;
  framedelay = 100;
  while (Serial.available()) {
    Serial.read();
  }
}


void parsecode() {
  int n = 0;
  nprog = 0;
  prog[nprog++] = n;
  while (nprog < NUM_PROG && n < NUM_CODE && code[n]) {
    if (code[n] == '+') prog[nprog++] = n+1;
    n++;
  }
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
*/
