#include "FastLED.h"
#include <ctype.h>

// #include "7172/7172.h"
#include "7172/cart.h"

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
char rpalv[RPAL_NUM] = "p|LOCs";
int rpaln = 6;

#define RFADE_NUM 60
char rfadev[RFADE_NUM];
uint8_t rfadet[RFADE_NUM];
uint8_t rfadet_min = 16;
uint8_t rfadet_max = 32;
int rfaden = 0;

#ifndef CODE_NUM
#define CODE_NUM 256
#endif
#ifndef PROG_NUM
#define PROG_NUM 64
#endif

#ifndef LED_CODE
char code[CODE_NUM] = 
    "+p/8s/8C/8O/8L/8|/8p !> "       // rainbow chase
    "+:%p|LOCs!%=600;"               // rainbow fades
    "+:%p|LOCsj8:d30!%60 "           // rainbow dots
    "+C "                            // blue
    "+?C6 !> "                       // blue chase 
    "+p "                            // red
    "+?p6 !> "                       // red chase
    "+L "                            // green
    "+?L6 !> "                       // green chase
    "+s "                            // magenta
    "+O "                            // cyan
    "+| "                            // yellow
    "+? "                            // white
    "+@ "                            // black
    "+p7?7C7 "                       // red white blue
    "+p7?7C7 !> "                    // red white blue chase
    "+:%xxx?,0!%500;"                // random white-on-gold pixels
    ;
#endif

int prog[PROG_NUM];
int progn = 0;
int progLast = 0;
int progNow = 0;
int pc = 0;
int pcstart = 0;
int frameMillis = 100;

#define MODE_PIN 2
#define KNOB_PIN A0
#define KNOB_NUM 3
#define KNOB_PROG 0
#define KNOB_BRIGHT 1
int knobv[KNOB_NUM] = { 0, 512, 0 };
int knobp = 0;


void setup() {
  delay(500);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(KNOB_PIN, INPUT_PULLUP);
  randomSeed(analogRead(1));
  Serial.begin(9600);
  Serial.setTimeout(250);
  FastLED.addLeds<WS2812B, LED_PIN, LED_ORDER>(ledv, LED_NUM);
  pal64();
  parseCode();
}

void loop() {
  if (Serial.available()) { readCode(); }
  knobControl();
  progControl();
  runCode();
  debugDisplay();
}


void knobControl() {
  static int modeLast = 0;
  static int knobLast = analogRead(KNOB_PIN);
  static long debounceNext = 0;
  int modeNow = digitalRead(MODE_PIN);
  int knobNow = analogRead(KNOB_PIN);
  long now = millis();
  
  if (modeNow != modeLast && now > debounceNext) {
    knobp = (knobp + modeNow) % KNOB_NUM;
    debounceNext = now + 50;
    modeLast = modeNow;
    knobLast = knobNow;
  }
  if (knobNow < knobLast - 5 || knobNow > knobLast + 5) {
    knobLast = -1000;
    knobv[knobp] = knobNow;
  }
}


void progControl() {
  progNow = (knobv[KNOB_PROG] * progn) / 1024;
  if (progNow != progLast) {
    progLast = progNow;
    pcstart = prog[progNow];
    progStart();
  }  
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


void progStart() {
  frameMillis = 100;
  rfadet_min = 16;
  rfadet_max = 32;
  frameStart();
}


void frameStart() {
  ledn = 0;
  rfaden = 0;
}

void runCode() {
  ledn = 0;
  pc = pcstart;
  while (pc < CODE_NUM && code[pc]) {
    if (Serial.available()) return;
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
      case '=':
        pc++; copyLast(); break;
      case '%':
        pc++; randomPixels(); break;
      case ':':
        pc++; colonCommand(); break;
      case ' ':
      case '\n':
        fillFrame();
        // fall through
      case ';':
        // display frame
        pc++;
        FastLED.setBrightness(knobv[KNOB_BRIGHT] / 4);
        FastLED.show();
        delay(frameMillis);
        frameStart();
        break;
      default:
        // fill pixels with a color
        if (c >= 0x3f && c <= 0x7f) {
          CRGB color = palette[c & 0x3f];
          for (int n = scanint(pc+1, 1); n > 0 && ledn < LED_NUM; n--)
            ledv[ledn++] = color;
        }
        else pc++;
        break;
    }
  }
}


void rampTo() {
  if (ledn > 0) {
    int n = scanint(pc, 1);
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


void copyLast() {
  if (ledn <= 0) return;
  for (int n = scanint(pc, 1); n > 0 && ledn < LED_NUM; n--) {
    ledv[ledn] = ledv[ledn-1];
    ledn++;
  }
}


void randomPixels() {
  for (int n = scanint(pc, 1); n > 0 && ledn < LED_NUM; n--) {
    if (rfaden < RFADE_NUM && rfadet_max > 1) {
      if (rfadet[rfaden] < 1) {
        rfadet[rfaden] = random(rfadet_min,rfadet_max);
        rfadev[rfaden] = rpalv[random8(rpaln)] & 0x3f;
      }
      nblend(ledv[ledn++], palette[rfadev[rfaden]], 255 / rfadet[rfaden]--);
      rfaden++;
    }
    else 
      ledv[ledn++] = palette[rpalv[random(rpaln)] & 0x3f];
  }
}


void colonCommand() {
  int c = code[pc];
  switch (c) {
    case '%': // set random palette
      pc++;
      if (code[pc] >= 0x3f && code[pc] <= 0x7f) {
        for(rpaln = 0; code[pc] >= 0x3f && code[pc] <= 0x7f; pc++) {
          if (rpaln < RPAL_NUM) rpalv[rpaln++] = code[pc];
        }
      }
      if (isdigit(code[pc]))
        rfadet_max = rfadet_min = scanint(pc, 16);
      if (code[pc] == ',')
        rfadet_max = scanint(pc+1, rfadet_min * 2);
      break;
    case 'd': // set frame delay
      frameMillis = scanint(pc+1, 50);
      break;
    default:
      pc++;
      break;
  }
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


void debugDisplay() {
  static long displayNext = 0;
  long now = millis();
  if (now > displayNext) {
    displayNext = now + 1000;
    Serial.print(knobp);
    for (int i = 0; i < KNOB_NUM; i++) {
      Serial.print(' '); Serial.print(knobv[i]);
    }
    Serial.print(" progNow=");
    Serial.println(progNow);
  }
}


void parseCode() {
  int n = (code[0] == '+');
  progn = 0;
  prog[progn++] = n;
  while (progn < PROG_NUM && n < CODE_NUM && code[n]) {
    if (code[n] == '+') prog[progn++] = n+1;
    n++;
  }
  progLast = -1;
}


void readCode() {
  // read a new code string from the serial port
  int n = Serial.readBytes(code, CODE_NUM - 1);
  code[n] = 0;
  while (Serial.available()) {
    Serial.read();
  }
  parseCode();
}

