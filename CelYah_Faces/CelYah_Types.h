#pragma once
#include <Arduino.h>

enum Style : uint8_t { P_NORM, P_NONE, P_STAR, P_HEART, P_SPIRAL, P_RING, P_ARC_UP, P_ARC_DN, P_CHEV_R, P_CHEV_L };
enum Fx : uint16_t {
  F_BLUSH = 1, F_TEAR = 2, F_TEARBIG = 4, F_SPARK = 8, F_ZZZ = 16, F_Q = 32,
  F_HEARTS = 64, F_THOUGHT = 128, F_LINES = 256, F_SHAKE = 512, F_JIGGLE = 1024, F_SWEAT = 2048
};

struct Eye { int8_t cover, tilt, px, py; uint8_t sz; uint8_t st; };
// cover: نسبة غطاء الجفن العلوي %  | tilt: ميلان الجفن (موجب = الطرف الداخلي أنزل = غاضب)
// px,py: اتجاه النظر | sz: حجم الحدقة | st: شكل الحدقة
struct Expr { const char *en; const char *ar; Eye l, r; uint16_t fx; };
