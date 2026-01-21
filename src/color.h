#include <stdint.h>
#include <stdbool.h>

static const char* color_skip_spaces(const char* s);
static int color_hex_value(int c);
static int parse_hex_byte(const char* s, uint8_t* out);
static int parse_u8_int(const char** ps, uint8_t* out);
static int color_expect_char(const char** ps, char ch);
static int color_parse_alpha(const char** ps, uint8_t* out);
static uint32_t color_pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
static int color_parse_hash_color(const char* s, uint32_t* out);
static int color_starts_with_icase(const char* s, const char* prefix);
static int color_parse_rgb_like(const char* raw, uint32_t* out);

bool color_parse(char* raw_color, uint32_t* out_color);