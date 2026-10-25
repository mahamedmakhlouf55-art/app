/*
  CelYah - 30 ملامح للروبوت (بدون كاميرا)
  اللوحة: ESP32-S3-CAM N16R8  |  الشاشة: ST7789 1.54" 240x240

  التوصيل (بورد ESP32-S3 بمنفذين USB-C):
    SCL -> IO12   SDA -> IO11   CS -> IO10   DC -> IO13   RST -> IO14
    BL/VCC -> 3V3   GND -> GND

  المكتبات (من Library Manager):
    Adafruit GFX Library
    Adafruit ST7735 and ST7789 Library

  إعدادات اللوحة في Arduino IDE:
    Board: ESP32S3 Dev Module
    PSRAM: OPI PSRAM
    Flash Size: 16MB

  التحكم:
    1) Serial Monitor (115200): اكتب رقم 1-30 أو الاسم (happy / سعيد)
       auto = تشغيل/إيقاف العرض التلقائي | next | prev
    2) الموبايل: وصّل على WiFi اسمه CelYah وباسورد 12345678
       وافتح المتصفح على 192.168.4.1
    3) من الكود: setExpr(n)  (n من 0 لـ 29)
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <WiFi.h>
#include <WebServer.h>
#include "CelYah_Types.h"

// ---------- الدبابيس ----------
#define PIN_SCL 12
#define PIN_SDA 11
#define PIN_DC  13
#define PIN_RST 14
#define PIN_CS  10

SPIClass tftSPI(FSPI);
Adafruit_ST7789 tft(&tftSPI, PIN_CS, PIN_DC, PIN_RST);
GFXcanvas16 *cv;
WebServer server(80);

// ---------- ألوان ----------
static inline uint16_t C(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
uint16_t COL_BODY, COL_GLOW, COL_LIGHT, COL_DARK, COL_WHITE, COL_PINK, COL_BLUE, COL_LINE, COL_SOFT;

// ---------- مقاسات ----------
const int EW = 100, EH = 108;
const int LEX = 10, REX = 130, EY0 = 66;

// ---------- تعريف الملامح ----------

#define E(c, t, x, y, s, st) { c, t, x, y, s, st }

const Expr EX[30] = {
  /* 1  */ {"normal",     "عادي",        E(0,0,0,0,30,P_NORM),   E(0,0,0,0,30,P_NORM),   0},
  /* 2  */ {"happy",      "سعيد",        E(0,0,0,0,0,P_ARC_UP),  E(0,0,0,0,0,P_ARC_UP),  0},
  /* 3  */ {"excited",    "متحمس",       E(0,0,0,0,32,P_STAR),   E(0,0,0,0,32,P_STAR),   F_SPARK},
  /* 4  */ {"love",       "محب",         E(0,0,0,0,32,P_HEART),  E(0,0,0,0,32,P_HEART),  F_HEARTS},
  /* 5  */ {"wink",       "غمز",         E(0,0,0,0,30,P_NORM),   E(0,0,0,0,0,P_ARC_UP),  0},
  /* 6  */ {"playful",    "مرح",         E(0,0,0,0,0,P_ARC_UP),  E(0,0,0,0,30,P_NORM),   0},
  /* 7  */ {"curious",    "فضولي",       E(0,0,0,0,38,P_NORM),   E(0,0,0,0,38,P_NORM),   0},
  /* 8  */ {"lookleft",   "ينظر لليسار", E(0,0,-45,0,30,P_NORM), E(0,0,-45,0,30,P_NORM), 0},
  /* 9  */ {"lookright",  "ينظر لليمين",        E(0,0,45,0,30,P_NORM),  E(0,0,45,0,30,P_NORM),  0},
  /* 10 */ {"lookup",     "ينظر لأعلى",        E(0,0,0,-45,30,P_NORM), E(0,0,0,-45,30,P_NORM), 0},
  /* 11 */ {"lookdown",   "ينظر لأسفل",        E(22,0,0,45,28,P_NORM), E(22,0,0,45,28,P_NORM), 0},
  /* 12 */ {"surprised",  "مندهش",       E(0,0,0,0,30,P_RING),   E(0,0,0,0,30,P_RING),   F_SPARK},
  /* 13 */ {"angry",      "غاضب",        E(25,28,0,8,30,P_NORM), E(25,28,0,8,30,P_NORM), 0},
  /* 14 */ {"grumpy",     "متضايق",      E(40,8,0,10,28,P_NORM), E(40,8,0,10,28,P_NORM), 0},
  /* 15 */ {"sad",        "حزين",        E(28,-22,0,12,30,P_NORM), E(28,-22,0,12,30,P_NORM), F_TEAR},
  /* 16 */ {"scared",     "خائف",        E(0,0,0,0,22,P_NORM),   E(0,0,0,0,22,P_NORM),   F_SHAKE | F_SWEAT},
  /* 17 */ {"sleepy",     "نعسان",       E(0,0,0,0,0,P_ARC_DN),  E(0,0,0,0,0,P_ARC_DN),  F_ZZZ},
  /* 18 */ {"bored",      "زهقان",       E(50,0,0,10,26,P_NORM), E(50,0,0,10,26,P_NORM), 0},
  /* 19 */ {"confused",   "مرتبك",       E(0,0,0,0,30,P_NORM),   E(10,-8,10,-10,30,P_NORM), F_Q},
  /* 20 */ {"dizzy",      "دوخان",       E(0,0,0,0,40,P_SPIRAL), E(0,0,0,0,40,P_SPIRAL), 0},
  /* 21 */ {"skeptical",  "مريب",        E(38,0,0,5,28,P_NORM),  E(28,16,0,5,28,P_NORM), 0},
  /* 22 */ {"mischievous","مشاغب",       E(30,20,0,6,28,P_NORM), E(30,20,0,6,28,P_NORM), F_SPARK},
  /* 23 */ {"shy",        "خجول",        E(0,0,-20,18,30,P_NORM),E(0,0,-20,18,30,P_NORM),F_BLUSH},
  /* 24 */ {"crying",     "يبكي",        E(0,0,0,5,38,P_NORM),   E(0,0,0,5,38,P_NORM),   F_TEARBIG},
  /* 25 */ {"laughing",   "يضحك",        E(0,0,0,0,0,P_CHEV_R),  E(0,0,0,0,0,P_CHEV_L),  F_JIGGLE},
  /* 26 */ {"focused",    "مركز",        E(38,16,0,6,24,P_NORM), E(38,16,0,6,24,P_NORM), 0},
  /* 27 */ {"proud",      "فخور",        E(0,0,0,-10,34,P_NORM), E(0,0,0,-10,34,P_NORM), F_SPARK},
  /* 28 */ {"thinking",   "يفكر",        E(0,0,35,-40,30,P_NORM),E(0,0,35,-40,30,P_NORM),F_THOUGHT},
  /* 29 */ {"shocked",    "مصدوم",       E(0,0,0,0,24,P_RING),   E(0,0,0,0,24,P_RING),   F_LINES},
  /* 30 */ {"adorable",   "لطيف",        E(0,0,0,0,0,P_ARC_UP),  E(0,0,0,0,0,P_ARC_UP),  F_HEARTS}
};

// ---------- حالة ----------
int curExpr = 0, pendingExpr = -1;
float blinkV = 0;           // 0 مفتوح .. 1 مقفول
int blinkPhase = 0;         // 0 ساكن | 1 بيقفل | 2 بيفتح
uint32_t nextBlink = 0, lastFrame = 0, lastAuto = 0;
float curOx[2] = {0, 0}, curOy[2] = {0, 0};
bool autoMode = true;

// ---------- رسم مساعد ----------
void heart(int x, int y, int s, uint16_t c) {
  int rr = max(2, s / 2);
  cv->fillCircle(x - rr, y, rr + 1, c);
  cv->fillCircle(x + rr, y, rr + 1, c);
  cv->fillTriangle(x - 2 * rr + 1, y + rr / 3, x + 2 * rr - 1, y + rr / 3, x, y + 2 * rr + rr / 3, c);
}

void star(int cx, int cy, int R, uint16_t c) {
  int xs[10], ys[10];
  float r = R * 0.45f;
  for (int i = 0; i < 10; i++) {
    float a = -PI / 2 + i * PI / 5;
    float rad = (i % 2 == 0) ? R : r;
    xs[i] = cx + cos(a) * rad;
    ys[i] = cy + sin(a) * rad;
  }
  for (int i = 0; i < 10; i++) cv->fillTriangle(cx, cy, xs[i], ys[i], xs[(i + 1) % 10], ys[(i + 1) % 10], c);
}

void spark(int x, int y, int s, uint16_t c) {
  cv->fillTriangle(x, y - s, x - s / 4, y, x + s / 4, y, c);
  cv->fillTriangle(x, y + s, x - s / 4, y, x + s / 4, y, c);
  cv->fillTriangle(x - s, y, x, y - s / 4, x, y + s / 4, c);
  cv->fillTriangle(x + s, y, x, y - s / 4, x, y + s / 4, c);
}

void thickArc(int cx, int cy, int r, int a0, int a1, int th, uint16_t c) {
  for (int a = a0; a <= a1; a += 3) {
    float rad = a * DEG_TO_RAD;
    cv->fillCircle(cx + cos(rad) * r, cy + sin(rad) * r, th / 2, c);
  }
}

void drop(int x, int y, int s, uint16_t c) {
  cv->fillCircle(x, y + s / 2, s / 2 + 1, c);
  cv->fillTriangle(x, y - s, x - s / 2 - 1, y + s / 2, x + s / 2 + 1, y + s / 2, c);
}

// ---------- رسم العين ----------
void drawEye(const Eye &e, float ox, float oy, bool left, int x0, int y0, float blink) {
  int cx = x0 + EW / 2, cy = y0 + EH / 2;

  // الجسم + التوهج + اللمعة
  cv->fillRoundRect(x0 - 3, y0 - 3, EW + 6, EH + 6, 43, COL_GLOW);
  cv->fillRoundRect(x0, y0, EW, EH, 40, COL_BODY);
  cv->fillRoundRect(x0 + 8, y0 + EH - 34, EW - 16, 26, 13, C(205, 140, 255));
  cv->fillRoundRect(x0 + 14, y0 + 9, 32, 14, 7, COL_LIGHT);
  cv->fillCircle(x0 + EW - 18, y0 + 18, 4, COL_LIGHT);

  // الحدقة / الرمز
  int pcx = cx + (int)ox, pcy = cy + (int)oy, r = e.sz;
  switch (e.st) {
    case P_NORM:
      cv->fillCircle(pcx, pcy, r, COL_DARK);
      cv->fillCircle(pcx - r / 3, pcy - r / 3, max(3, r / 4), COL_WHITE);
      cv->fillCircle(pcx + r / 3, pcy + r / 2, max(2, r / 9), COL_PINK);
      break;
    case P_STAR:
      cv->fillCircle(pcx, pcy, r, COL_DARK);
      star(pcx, pcy + 1, (int)(r * 0.78f), C(255, 235, 255));
      break;
    case P_HEART:
      cv->fillCircle(pcx, pcy, r, COL_DARK);
      heart(pcx, pcy - 4, (int)(r * 0.62f), C(255, 120, 210));
      break;
    case P_SPIRAL: {
      cv->fillCircle(pcx, pcy, r, COL_DARK);
      float rot = millis() / 160.0f;
      for (float a = 0; a < 17.0f; a += 0.12f) {
        float rr = a * 1.9f;
        cv->fillCircle(pcx + cos(a + rot) * rr, pcy + sin(a + rot) * rr, 2, C(200, 160, 255));
      }
      break;
    }
    case P_RING:
      cv->fillCircle(pcx, pcy, r, COL_DARK);
      cv->fillCircle(pcx, pcy, (int)(r * 0.62f), C(225, 175, 255));
      cv->fillCircle(pcx, pcy, (int)(r * 0.34f), COL_DARK);
      break;
    case P_ARC_UP:
      thickArc(cx, cy + 14, 32, 205, 335, 9, COL_DARK);
      break;
    case P_ARC_DN:
      thickArc(cx, cy - 14, 32, 25, 155, 9, COL_DARK);
      break;
    case P_CHEV_R:
      for (int d = -3; d <= 3; d++) {
        cv->drawLine(cx - 22, cy - 28 + d, cx + 22, cy + d, COL_DARK);
        cv->drawLine(cx + 22, cy + d, cx - 22, cy + 28 + d, COL_DARK);
      }
      break;
    case P_CHEV_L:
      for (int d = -3; d <= 3; d++) {
        cv->drawLine(cx + 22, cy - 28 + d, cx - 22, cy + d, COL_DARK);
        cv->drawLine(cx - 22, cy + d, cx + 22, cy + 28 + d, COL_DARK);
      }
      break;
    default: break;
  }

  // الجفن العلوي (غضب / حزن / زهق / رمشة)
  float cp = max((float)e.cover, blink * 100.0f);
  if (cp > 0.5f) {
    int c = (int)(cp * EH / 100.0f);
    int t = (int)(e.tilt * (1.0f - blink));
    int xo = left ? x0 - 3 : x0 + EW + 3;
    int xi = left ? x0 + EW + 3 : x0 - 3;
    int yo = max(y0 + c - t, y0 - 6);
    int yi = max(y0 + c + t, y0 - 6);
    cv->fillTriangle(xo, y0 - 6, xi, y0 - 6, xi, yi, 0);
    cv->fillTriangle(xo, y0 - 6, xi, yi, xo, yo, 0);
    // خط الجفن
    float t0 = 0.07f, t1 = 0.93f;
    int ax = xo + (xi - xo) * t0, ay = yo + (yi - yo) * t0;
    int bx = xo + (xi - xo) * t1, by = yo + (yi - yo) * t1;
    for (int d = 0; d < 4; d++) cv->drawLine(ax, ay + d, bx, by + d, COL_LINE);
  }
}

// ---------- المؤثرات ----------
void drawFx(const Expr &e, int sx, int sy) {
  uint32_t t = millis();
  uint16_t f = e.fx;

  if (f & F_BLUSH) {
    for (int i = 0; i < 3; i++) {
      cv->drawLine(LEX + 6 + i * 9 + sx, EY0 + EH - 14 + sy, LEX + 12 + i * 9 + sx, EY0 + EH - 24 + sy, COL_PINK);
      cv->drawLine(REX + EW - 34 + i * 9 + sx, EY0 + EH - 14 + sy, REX + EW - 28 + i * 9 + sx, EY0 + EH - 24 + sy, COL_PINK);
    }
  }
  if (f & F_TEAR) {
    int ph = (t / 45) % 36;
    drop(LEX + EW - 24 + sx, EY0 + EH - 6 + ph, 9, COL_BLUE);
    drop(REX + 24 + sx, EY0 + EH - 6 + ((ph + 14) % 36), 9, COL_BLUE);
  }
  if (f & F_TEARBIG) {
    int b = (int)(3 * sin(t / 180.0f));
    cv->fillCircle(LEX + 36, EY0 + EH + 2 + b, 15, COL_BLUE);
    cv->fillCircle(LEX + 62, EY0 + EH + 14 + b, 9, COL_BLUE);
    cv->fillCircle(REX + 64, EY0 + EH + 2 - b, 15, COL_BLUE);
    cv->fillCircle(REX + 38, EY0 + EH + 14 - b, 9, COL_BLUE);
  }
  if (f & F_SPARK) {
    int s1 = 9 + (int)(3 * sin(t / 200.0f)), s2 = 9 + (int)(3 * cos(t / 230.0f));
    spark(18 + sx, 48 + sy, s1, C(255, 230, 255));
    spark(222 + sx, 44 + sy, s2, C(255, 230, 255));
  }
  if (f & F_ZZZ) {
    cv->setTextColor(C(230, 190, 255));
    int b = (int)(3 * sin(t / 300.0f));
    cv->setTextSize(2); cv->setCursor(168, 54 + b); cv->print("z");
    cv->setTextSize(3); cv->setCursor(188, 32 + b); cv->print("Z");
    cv->setTextSize(4); cv->setCursor(210, 6 + b);  cv->print("Z");
  }
  if (f & F_Q) {
    cv->setTextColor(C(235, 200, 255));
    cv->setTextSize(6);
    cv->setCursor(196, 4 + (int)(3 * sin(t / 250.0f)));
    cv->print("?");
  }
  if (f & F_HEARTS) {
    int b = (int)(3 * sin(t / 220.0f));
    heart(26 + sx, 40 + sy + b, 12, COL_PINK);
    heart(212 + sx, 28 + sy - b, 14, COL_PINK);
    heart(224 + sx, 60 + sy + b, 8, C(255, 150, 220));
  }
  if (f & F_THOUGHT) {
    cv->fillCircle(172, 52, 4, C(225, 190, 255));
    cv->fillCircle(186, 40, 6, C(225, 190, 255));
    cv->fillRoundRect(190, 8, 46, 30, 14, C(225, 190, 255));
  }
  if (f & F_LINES) {
    for (int i = 0; i < 4; i++) {
      int h = 22 + (i % 2) * 10;
      cv->drawFastVLine(LEX + 18 + i * 20, 8, h, C(210, 150, 255));
      cv->drawFastVLine(REX + 18 + i * 20, 8, h, C(210, 150, 255));
    }
  }
  if (f & F_SWEAT) {
    drop(222 + sx, 40 + sy + (int)(3 * sin(t / 150.0f)), 10, COL_BLUE);
  }
}

// ---------- الرسم الكامل ----------
void render() {
  cv->fillScreen(0);
  const Expr &e = EX[curExpr];

  int sx = 0, sy = 0;
  if (e.fx & F_SHAKE)  { sx = random(-3, 4); sy = random(-2, 3); }
  if (e.fx & F_JIGGLE) { sx = random(-1, 2); sy = random(-2, 3); }

  const Eye *eyes[2] = {&e.l, &e.r};
  for (int i = 0; i < 2; i++) {
    float tx = eyes[i]->px * 0.4f, ty = eyes[i]->py * 0.4f;
    curOx[i] += (tx - curOx[i]) * 0.3f;
    curOy[i] += (ty - curOy[i]) * 0.3f;
  }
  drawEye(e.l, curOx[0], curOy[0], true,  LEX + sx, EY0 + sy, blinkV);
  drawEye(e.r, curOx[1], curOy[1], false, REX + sx, EY0 + sy, blinkV);
  drawFx(e, sx, sy);

  tft.drawRGBBitmap(0, 0, cv->getBuffer(), 240, 240);
}

// ---------- التحكم في الملامح ----------
bool isBlinkable(const Eye &x) {
  return x.st == P_NORM || x.st == P_STAR || x.st == P_HEART || x.st == P_RING;
}

void setExpr(int n) {              // 0..29
  if (n < 0 || n >= 30) return;
  pendingExpr = n;
  blinkPhase = 1;                  // رمشة سريعة وبعدين يتغير الوجه
}

void updateBlink() {
  uint32_t now = millis();
  if (blinkPhase == 0) {
    if (now > nextBlink) {
      if (isBlinkable(EX[curExpr].l) && isBlinkable(EX[curExpr].r)) blinkPhase = 1;
      nextBlink = now + random(2500, 5500);
    }
    return;
  }
  if (blinkPhase == 1) {
    blinkV += 0.34f;
    if (blinkV >= 1.0f) {
      blinkV = 1.0f;
      if (pendingExpr >= 0) { curExpr = pendingExpr; pendingExpr = -1; }
      blinkPhase = 2;
    }
  } else {
    blinkV -= 0.25f;
    if (blinkV <= 0) { blinkV = 0; blinkPhase = 0; }
  }
}

int findByName(String s) {
  s.trim(); s.toLowerCase();
  if (s.length() == 0) return -1;
  int n = s.toInt();
  if (n >= 1 && n <= 30) return n - 1;
  for (int i = 0; i < 30; i++) {
    String en = EX[i].en, ar = EX[i].ar;
    if (s == en || s == ar) return i;
  }
  return -1;
}

void handleSerial() {
  if (!Serial.available()) return;
  String s = Serial.readStringUntil('\n');
  s.trim();
  if (s.length() == 0) return;
  String l = s; l.toLowerCase();
  if (l == "auto") { autoMode = !autoMode; Serial.println(autoMode ? "auto ON" : "auto OFF"); return; }
  if (l == "next") { autoMode = false; setExpr((curExpr + 1) % 30); return; }
  if (l == "prev") { autoMode = false; setExpr((curExpr + 29) % 30); return; }
  int n = findByName(s);
  if (n >= 0) {
    autoMode = false;
    setExpr(n);
    Serial.printf("%d: %s | %s\n", n + 1, EX[n].en, EX[n].ar);
  } else {
    Serial.println("?? اكتب رقم 1-30 او اسم الملمح");
  }
}

// ---------- صفحة الموبايل ----------
void handleRoot() {
  String h = F("<!doctype html><html dir='rtl'><head><meta charset='utf-8'>"
               "<meta name='viewport' content='width=device-width,initial-scale=1'>"
               "<style>body{background:#0b0b12;color:#fff;font-family:sans-serif;text-align:center}"
               "button{margin:5px;padding:12px 16px;border:0;border-radius:14px;background:#a45cff;color:#fff;font-size:18px}"
               "h2{color:#d9b3ff}</style></head><body><h2>CelYah</h2>");
  for (int i = 0; i < 30; i++) {
    h += "<button onclick=\"fetch('/e?n=" + String(i + 1) + "')\">" + String(EX[i].ar) + "</button>";
  }
  h += F("<br><br><button onclick=\"fetch('/auto')\" style='background:#555'>تلقائي</button></body></html>");
  server.send(200, "text/html; charset=utf-8", h);
}

void handleExpr() {
  int n = server.arg("n").toInt();
  autoMode = false;
  setExpr(n - 1);
  server.send(200, "text/plain", "ok");
}

void handleAuto() {
  autoMode = !autoMode;
  server.send(200, "text/plain", "ok");
}

// ---------- setup / loop ----------
void setup() {
  Serial.begin(115200);
  delay(300);

  COL_BODY  = C(185, 105, 250);
  COL_GLOW  = C(95, 30, 165);
  COL_LIGHT = C(235, 195, 255);
  COL_DARK  = C(35, 0, 65);
  COL_WHITE = C(255, 255, 255);
  COL_PINK  = C(255, 130, 210);
  COL_BLUE  = C(110, 140, 255);
  COL_LINE  = C(70, 15, 125);

  tftSPI.begin(PIN_SCL, -1, PIN_SDA, -1);
  tft.init(240, 240, SPI_MODE3);      // لو الصورة بوظت جرب SPI_MODE2 او SPI_MODE0
  tft.setSPISpeed(40000000);          // لو فيه تشويش قلله لـ 27000000
  tft.setRotation(1);
  tft.fillScreen(0);

  cv = new GFXcanvas16(240, 240);
  if (!cv || !cv->getBuffer()) {
    Serial.println("Canvas alloc failed!");
    while (true) delay(1000);
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP("CelYah", "12345678");
  server.on("/", handleRoot);
  server.on("/e", handleExpr);
  server.on("/auto", handleAuto);
  server.begin();
  Serial.print("Web: http://");
  Serial.println(WiFi.softAPIP());

  nextBlink = millis() + 3000;
  Serial.println("اكتب رقم 1-30 او اسم الملمح | auto | next | prev");
}

void loop() {
  server.handleClient();
  handleSerial();

  if (autoMode && millis() - lastAuto > 3500) {
    lastAuto = millis();
    setExpr((curExpr + 1) % 30);
  }

  if (millis() - lastFrame >= 33) {
    lastFrame = millis();
    updateBlink();
    render();
  }
}
