#pragma once
#include <stdint.h>
#include <string>
#include <cstring>

// ── Basic Arduino types ───────────────────────────────────────────────────
typedef bool     boolean;
using    String = std::string;

// ── Pin / level constants ─────────────────────────────────────────────────
#define HIGH   1
#define LOW    0
#define INPUT  0
#define OUTPUT 1

// F() stores strings in flash on AVR/ESP; on native it is a no-op.
#define F(x) (x)

// ── Controllable mock state (defined once per test binary) ────────────────
// Each test file that includes this header must define these:
//   uint32_t  _mock_millis = 0;
//   int       _mock_analog[256] = {};
//   MockSerial Serial;
extern uint32_t  _mock_millis;
extern int       _mock_analog[256];

inline uint32_t millis()            { return _mock_millis; }
inline void     advance_time(uint32_t ms) { _mock_millis += ms; }
inline int      analogRead(int pin) { return _mock_analog[pin]; }
inline void     set_analog(int pin, int val) { _mock_analog[pin] = val; }

inline void digitalWrite(int /*pin*/, int /*val*/) {}
inline void pinMode(int /*pin*/, int /*mode*/)     {}
inline void tone(int /*pin*/, int /*freq*/)         {}
inline void noTone(int /*pin*/)                     {}

// ── Minimal Serial mock ───────────────────────────────────────────────────
struct MockSerial {
    template<typename T> void print(T)   {}
    template<typename T> void println(T) {}
};
extern MockSerial Serial;
