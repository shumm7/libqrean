#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char* color_skip_spaces(const char* s) {
  while (*s && isspace((unsigned char)*s)) s++;
  return s;
}

static int color_hex_value(int c) {
  if ('0' <= c && c <= '9') return c - '0';
  if ('a' <= c && c <= 'f') return 10 + (c - 'a');
  if ('A' <= c && c <= 'F') return 10 + (c - 'A');
  return -1;
}

static int parse_hex_byte(const char* s, uint8_t* out) {
  int h1 = color_hex_value((unsigned char)s[0]);
  int h2 = color_hex_value((unsigned char)s[1]);
  if (h1 < 0 || h2 < 0) return 0;
  *out = (uint8_t)((h1 << 4) | h2);
  return 1;
}

static int parse_u8_int(const char** ps, uint8_t* out) {
  const char* s = color_skip_spaces(*ps);
  if (!isdigit((unsigned char)*s)) return 0;

  errno = 0;
  char* end = NULL;
  long v = strtol(s, &end, 10);
  if (end == s || errno != 0) return 0;
  if (v < 0 || v > 255) return 0;

  *out = (uint8_t)v;
  *ps = end;
  return 1;
}

static int color_expect_char(const char** ps, char ch) {
  const char* s = color_skip_spaces(*ps);
  if (*s != ch) return 0;
  *ps = s + 1;
  return 1;
}

static int color_parse_alpha(const char** ps, uint8_t* out) {
  const char* s = color_skip_spaces(*ps);
  if (!*s) return 0;

  if (*s == '.' || *s == '+' || *s == '-' || isdigit((unsigned char)*s)) {
    const char* t = s;
    while (*t && (isdigit((unsigned char)*t) || isspace((unsigned char)*t))) t++;
    if (*t != '.') {
      return parse_u8_int(ps, out);
    }

    errno = 0;
    char* end = NULL;
    double v = strtod(s, &end);
    if (end == s || errno != 0) return 0;
    if (v < 0.0 || v > 1.0) return 0;

    int a = (int)(v * 255.0 + 0.5);
    if (a < 0) a = 0;
    if (a > 255) a = 255;

    *out = (uint8_t)a;
    *ps = end;
    return 1;
  }

  return 0;
}

static uint32_t color_pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
}

static int color_parse_hash_color(const char* s, uint32_t* out) {
  size_t n = 0;
  while (isxdigit((unsigned char)s[n])) n++;

  const char* tail = s + n;
  tail = color_skip_spaces(tail);
  if (*tail != '\0') return 0;

  if (n != 6 && n != 8) return 0;

  uint8_t r, g, b, a = 0xFF;
  if (!parse_hex_byte(s + 0, &r)) return 0;
  if (!parse_hex_byte(s + 2, &g)) return 0;
  if (!parse_hex_byte(s + 4, &b)) return 0;
  if (n == 8) {
    if (!parse_hex_byte(s + 6, &a)) return 0;
  }
  *out = color_pack_rgba(r, g, b, a);
  return 1;
}

static int color_starts_with_icase(const char* s, const char* prefix) {
  while (*prefix) {
    if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix)) return 0;
    s++; prefix++;
  }
  return 1;
}

static int color_parse_rgb_like(const char* raw, uint32_t* out) {
  const char* s = color_skip_spaces(raw);

  int has_alpha = 0;
  if (color_starts_with_icase(s, "rgb(")) {
    s += 4; // "rgb("
    has_alpha = 0;
  } else if (color_starts_with_icase(s, "rgba(")) {
    s += 5; // "rgba("
    has_alpha = 1;
  } else {
    return 0;
  }

  uint8_t r, g, b, a = 0xFF;
  if (!parse_u8_int(&s, &r)) return 0;
  if (!color_expect_char(&s, ',')) return 0;
  if (!parse_u8_int(&s, &g)) return 0;
  if (!color_expect_char(&s, ',')) return 0;
  if (!parse_u8_int(&s, &b)) return 0;

  if (has_alpha) {
    if (!color_expect_char(&s, ',')) return 0;
    if (!color_parse_alpha(&s, &a)) return 0;
  }

  if (!color_expect_char(&s, ')')) return 0;

  s = color_skip_spaces(s);
  if (*s != '\0') return 0;

  *out = color_pack_rgba(r, g, b, a);
  return 1;
}

bool color_parse(char* raw_color, uint32_t* out) {
  if (!raw_color) return false;

  const char* s = color_skip_spaces(raw_color);
  if (*s == '#') {
    uint32_t v = 0;
    if (color_parse_hash_color(s + 1, &v)){
      *out = v;
      return true;
    }
  }else{
    uint32_t v = 0;
    if (color_parse_rgb_like(raw_color, &v)){
      *out = v;
      return true;
    }
  }

  return false;
}