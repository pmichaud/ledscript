#include "FastLED.h"
#include <ctype.h>

#define LED_NUM 60
#define LED_PIN 4
#define LED_ORDER GRB
#define PALETTE_NUM 64
#define RPAL_NUM 32
#define RFADE_NUM 60
#define CODE_NUM 256
#define CLIP_NUM 32
#define MODE_PIN 2
#define AUX_PIN 3
#define KNOB_PIN A0
#define KNOB_LED 0
#define SETUP() ;

enum { p_framemsec, p_palette, p_fsize, p_foffset, p_rfadet_min, p_rfadet_max, p_rfadeq_min, p_rfadeq_max, PARAM_NUM };

#include "7172/7172.h"
// #include "7172/cart.h"
// #include "7172/tophat.h"

int paramc[PARAM_NUM];                     // current clip parameters
int paramd[PARAM_NUM] =                    // default clip parameters
    { 100, 0, LED_NUM, 0, 16, 32, 0, 0 };

CRGB  ledv[LED_NUM];                       // vector of led values
CRGB* ledf = ledv;                         // start of led drawing frame
int   ledmax = LED_NUM;                    // end of led drawing frame
int   ledn = 0;                            // number of leds drawn
int   ledfill = 0;                         // size of fill repeat pattern

CRGB    palette[PALETTE_NUM];              // color palette
char    rpalv[RPAL_NUM] = "BCDEFGHI";      // ramp palette
uint8_t rpaln = 8;                         // number of entries in ramp

uint8_t rfadev[RFADE_NUM];
uint8_t rfadet[RFADE_NUM];
int     rfaden = 0;
int&    rfadet_min = paramc[p_rfadet_min];
int&    rfadet_max = paramc[p_rfadet_max];

#ifndef LED_CODE
char code[CODE_NUM] =
    "+B/5C/5D/8E/8F/8G/8H/8I/8B !> " // rainbow chase
    "+:%ABCDEFGHI!%=600;"            // rainbow fades
    "+:%ABCDEFGHI8,:d30!%60 "        // rainbow dots
    "+B "                            // red
    "+AB6 !> "                       // red chase
    "+G "                            // blue
    "+AG6 !> "                       // blue chase
    "+E "                            // green
    "+AE6 !> "                       // green chase
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

long    knobledUntil = 0;
enum    { k_clip, k_bright, k_debug, KNOB_NUM };
int     knobr[KNOB_NUM] = { 1, 16, 1 };
int     knobv[KNOB_NUM] = { 0, 8, 0 };
uint8_t knobp = 0;

int     clip[CLIP_NUM];
int     subclip[8];
int&    clipn = knobr[k_clip];
uint8_t clipLast = 0;
uint8_t clipNow = 0;
uint8_t subclipn = 0;
int     pc = 0;
int     pcstart = 0;
int&    framemsec = paramc[p_framemsec];


void setup() {
  delay(500);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(AUX_PIN, INPUT_PULLUP);
  pinMode(KNOB_PIN, INPUT_PULLUP);
  randomSeed(analogRead(1));
  Serial.begin(9600);
  Serial.setTimeout(250);
  FastLED.addLeds<WS2812B, LED_PIN, LED_ORDER>(ledv, LED_NUM);
  palRainbow();
  parseCode();
  SETUP();
}

void loop() {
  if (Serial.available()) { readCode(); }
  knobControl();
  clipControl();
  runCode();
  debugDisplay();
}


void knobControl() {
  static int modeLast = digitalRead(MODE_PIN);
  static int auxLast = digitalRead(AUX_PIN);
  static int knobLast = analogRead(KNOB_PIN);
  static long debounceUntil = 0;
  int modeNow = digitalRead(MODE_PIN);
  int auxNow = digitalRead(AUX_PIN);
  int knobNow = analogRead(KNOB_PIN);
  long now = millis();
  
  if (modeNow != modeLast && now > debounceUntil) {
    knobp = (knobp + modeNow) % KNOB_NUM;
    knobledUntil = now + 1000;
    debounceUntil = now + 50;
    modeLast = modeNow;
    knobLast = knobNow;
  }
  if (auxNow != auxLast && now > debounceUntil) {
    knobv[knobp] = (knobv[knobp] + auxNow) % knobr[knobp];
    knobledUntil = now + 1000;
    debounceUntil = now + 50;
    auxLast = auxNow;
  }
  if (knobNow < knobLast - 8 || knobNow > knobLast + 8) {
    knobv[knobp] = ((long)knobNow * knobr[knobp]) / 1024;
    knobLast = knobNow;
    knobledUntil = now + 1000;
  }
}


void clipControl() {
  clipNow = knobv[k_clip];
  if (clipNow != clipLast) {
    clipLast = clipNow;
    pcstart = clip[clipNow];
    clipStart();
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


void clipStart() {
  for (int i = 0; i < PARAM_NUM; i++) {
    paramc[i] = paramd[i];
  }
  subclipn = 0;
  frameStart();
}

void frameStart() {
  frameSet();
  ledn = 0;
  rfaden = 0;
}

void frameSet() {
  ledf = ledv + paramc[p_foffset];
  ledmax = min(paramc[p_fsize], LED_NUM - paramc[p_foffset]);
}

void runCode() {
  int n;
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
      case '.':
        n = scanint(pc+1, 1);
        ledn += n;
        if (ledn > ledmax) ledn = ledmax;
        break;
      case '(':
        n = scanint(pc+1, 1);       
        if (subclipn < 8) {
          subclip[subclipn++] = pc;
          subclip[subclipn++] = n-1;
        }
        break; 
      case ')':
        pc++;
        if (subclipn > 0) {
          if (subclip[subclipn-1]-- < 1) subclipn -= 2;
          else pc = subclip[subclipn-2];
        }
        break;
      case ' ':
        fillFrame();
        // fall through
      case ';':
        // display frame
        ledfill = ledn;
        pc++;
        FastLED.setBrightness(max(knobv[k_bright] * 17, 3));
        if (millis() > knobledUntil) FastLED.show();
        else {
          CRGB t = ledv[KNOB_LED];                    // note ledv[0] not ledf[0]
          ledv[KNOB_LED] = palette[knobp];
          FastLED.show();
          ledv[KNOB_LED] = t;
        }
        delay(framemsec);
        knobControl();
        frameStart();
        break;
      default:
        // fill pixels with a color
        if (c >= 0x3f && c <= 0x7f) {
          CRGB color = palette[c & 0x3f];
          for (int n = scanint(pc+1, 1); n > 0 && ledn < ledmax; n--)
            ledf[ledn++] = color;
        }
        else pc++;
        break;
    }
  }
}


void rampTo() {
  if (ledn > 0) {
    int n = scanint(pc, 1);
    CRGB from = ledf[ledn - 1];
    CRGB to = (code[pc]) ? palette[code[pc++] & 0x3f] : CRGB::Black;
    int r0 = from.r;  int rd = to.r - r0;
    int g0 = from.g;  int gd = to.g - g0;
    int b0 = from.b;  int bd = to.b - b0;
    for (int i = 1; i <= n && ledn < ledmax; i++) {
      ledf[ledn++] = CRGB(r0+(rd*i)/n, g0+(gd*i)/n, b0+(bd*i)/n); 
    }
  }
}


void fillFrame() {
  if (ledn > 0) {
    ledfill = ledn;
    for (int j = ledfill; j < ledmax; j++) {
      ledf[j] = ledf[j - ledfill];
    }
  }
}


void rotateLeft() {
  int n = ledfill;
  if (ledn + n > ledmax) n = ledmax - ledn;
  CRGB t = ledf[ledn];
  for (int i = 0; i < n - 1; i++) ledf[ledn+i] = ledf[ledn+i+1];
  ledf[ledn+n-1] = t;
  ledn += n;
}


void rotateRight() {
  int n = ledfill;
  if (ledn + n > ledmax) n = ledmax - ledn;
  CRGB t = ledf[ledn + n - 1];
  for (int i = n-1; i > 0; i--) ledf[ledn+i] = ledf[ledn+i-1];
  ledf[ledn] = t;
  ledn += n;
}


void shiftLeft() {
  int n = ledfill;
  if (ledn + n > ledmax) n = ledmax - ledn;
  for (int i = 0; i < n - 1; i++) ledf[ledn+i] = ledf[ledn+i+1];
  ledn += n;
}


void shiftRight() {
  int n = ledfill;
  if (ledn + n > ledmax) n = ledmax - ledn;
  for (int i = n-1; i > 0; i--) ledf[ledn+i] = ledf[ledn+i-1];
  ledn += n;
}


void copyLast() {
  if (ledn <= 0) return;
  for (int n = scanint(pc, 1); n > 0 && ledn < ledmax; n--) {
    ledf[ledn] = ledf[ledn-1];
    ledn++;
  }
}


void randomPixels() {
  for (int n = scanint(pc, 1); n > 0 && ledn < ledmax; n--) {
    if (rfaden < RFADE_NUM && rfadet_max > 0) {
      uint8_t& rv = rfadev[rfaden];
      uint8_t& rt = rfadet[rfaden];
      if (rfadet[rfaden] < 1) {
        int& rfadeq_min = paramc[p_rfadeq_min];
        int& rfadeq_max = paramc[p_rfadeq_max];
        rv += random(rfadeq_min, (rfadeq_max == 0 && rfadeq_min == 0) ? rpaln : rfadeq_max + 1);
        rv %= rpaln;
        rt = random(rfadet_min, rfadet_max + 1);
      }
      nblend(ledf[ledn++], palette[rpalv[rv] & 0x3f], 255 / rt--);
      rfaden++;
    }
    else 
      ledf[ledn++] = palette[rpalv[random(rpaln)] & 0x3f];
  }
}


int setParam(int p, int v, int isGlobal = 0, int vmin = -32768, int vmax = 32767) {
  v = constrain(v, vmin, vmax);
  if (p >= 0 && p < PARAM_NUM) {
    paramc[p] = v;
    if (isGlobal) paramd[p] = v;
  }
  return v;
}

void colonCommand() {
  int c = code[pc];
  int v;
  switch (c) {
    case '%': // set random palette
      pc++;
      if (code[pc] >= 0x3f && code[pc] <= 0x7f) {
        for(rpaln = 0; code[pc] >= 0x3f && code[pc] <= 0x7f; pc++) {
          if (rpaln < RPAL_NUM) rpalv[rpaln++] = code[pc];
        }
      }
      rfadet_min = 16; rfadet_max = 32;
      if (isdigit(code[pc])) {
        setParam(p_rfadet_min, scanint(pc, 16), 1, 0, 255);
        setParam(p_rfadet_max, rfadet_min, 1, 0, 255);
      }
      if (code[pc] == ',')
        setParam(p_rfadet_max, scanint(pc+1, rfadet_min * 2), 1, rfadet_min, 255);
      break;
    case 'q': // set fade randomizer
      v = setParam(p_rfadeq_min, scanint(pc+1, 0), isupper(c));
      if (code[pc] == ',') v = scanint(pc+1, v);
      setParam(p_rfadeq_max, v, isupper(c));
      break;
    case 'd': // set frame delay
      setParam(p_framemsec, scanint(pc+1, 100), isupper(c), 0);
      break;
    case 'n': // set frame size
    case 'N':
      setParam(p_fsize, scanint(pc+1, LED_NUM), isupper(c), 0);
      frameSet();
      break;
    case 'o': // set frame offset
    case 'O':
      setParam(p_foffset, scanint(pc+1, LED_NUM), isupper(c), 0);
      frameSet();
      break;
    case 'p':
      displayPalette();
      for (int i = 0; i < PALETTE_NUM && ledn < ledmax; i++)
        ledf[ledn++] = palette[i];
      break;
    case '!':
      delay(scanint(pc+1, 10)*1000);
      break;
    case '<':
      pc++; shiftLeft(); break;
    case '>':
      pc++; shiftRight(); break;
    default:
      pc++;
      break;
  }
}


void debugDisplay() {
/* 
  static long displayNext = 0;
  long now = millis();
  if (now > displayNext) {
    displayNext = now + 1000;
    Serial.print(knobp);
    for (int i = 0; i < KNOB_NUM; i++) {
      Serial.print(' '); Serial.print(knobv[i]);
    }
    Serial.print(" clipNow=");
    Serial.println(clipNow);
  }
*/
}

void parseCode() {
  int n = (code[0] == '+');
  clipn = 0;
  clip[clipn++] = n;
  while (clipn < CLIP_NUM && n < CODE_NUM && code[n]) {
    if (code[n] == '+') clip[clipn++] = n+1;
    n++;
  }
  clipLast = -1;
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


void palRainbow() {
  palette[0] = 0;
  palette[1] = 0xababab;
  palette[2] = 0x222222;
  palette[32] = CHSV(0, 0, 51);
  palette[33] = 0xffffff;
  for (int i = 0; i < 8; i++) {
    int hue = i * 32;
    palette[i+2] = CHSV(hue, 255, 255);
    palette[i+10] = CHSV(hue, 255, 128);
    palette[i+18] = CHSV(hue, 255, 51);
    palette[i+34] = CHSV(hue, 153, 255);
    palette[i+42] = CHSV(hue, 153, 128);
  }
  palette[27] = 0xFF8000;
  palette[28] = 0xFFFF00;
  palette[29] = 0x00FFFF;
  palette[30] = 0x8000FF;
  palette[31] = 0xFF00FF;
}


void pal64() {
  // initialize palette with 6-bit color palette
  uint8_t v[4] = { 0x00, 0x22, 0x77, 0xff };
  int n = 0;
  for (int r = 0; r < 4; r++) {
    for (int g = 0; g < 4; g++) {
      for (int b = 0; b < 4; b++) {
        palette[n++] = CRGB(v[r], v[g], v[b]);
      }
    }
  }
}


void displayPalette() {
/*  
  // display current palette settings
  char buf[80];
  for (int i = 0; i < PALETTE_NUM; i++) {
    CRGB c = palette[i];
    sprintf(buf, "%2d %c: %02x%02x%02x", i, i+'@', c.r, c.g, c.b);
    Serial.println(buf);
  }
*/
}


