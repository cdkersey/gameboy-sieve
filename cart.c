typedef unsigned char tile_t[16];
typedef unsigned char map_row_t[32];

typedef struct {
  unsigned char y, x, tile, flags;
} sprite_t;

#define PAL_LIGHT 0x50
#define PAL_NORMAL 0xe4
#define PAL_DARK 0xfa
#define PAL_BLACK 0xff

#define BGPOSY ((unsigned char*)0xff42)
#define BGPOSX ((unsigned char*)0xff43)
#define WNDPOSY ((unsigned char*)0xff4a)
#define WNDPOSX ((unsigned char*)0xff4b)

#define BGPAL ((unsigned char*)0xff47)
#define SPRITEPAL0 ((unsigned char*)0xff48)
#define SPRITEPAL1 ((unsigned char*)0xff49)
#define LCDCONT ((volatile unsigned char*)0xff40)
#define LCDSTAT ((volatile unsigned char*)0xff41)
#define LO_MAP ((map_row_t*)0x9800)
#define HI_MAP ((map_row_t*)0x9c00)
#define LO_TILES ((tile_t*)0x8000)
#define HI_TILES ((tile_t*)0x8c00)
#define SPRITES ((sprite_t *)0xfe00)

#define IRQEN ((unsigned char *)0xffff)

enum LCDCONT_BIT {
  LCD_ENABLE = 0x80,
  LCD_WSEL = 0x40,
  LCD_WINDOW = 0x20,
  LCD_BGTILES = 0x10,
  LCD_BGSEL = 0x08,
  LCD_TRWIN = 0x02,
  LCD_BG = 0x01
};

void disable_lcd(void) { *LCDCONT &= ~LCD_ENABLE; }
void enable_lcd(void) { *LCDCONT |= LCD_ENABLE; }

void disable_bg(void) { *LCDCONT &= ~LCD_BG; }
void enable_bg(void) { *LCDCONT |= LCD_BG; }

void disable_window(void) { *LCDCONT &= ~LCD_WINDOW; }
void enable_window(void) { *LCDCONT |= LCD_WINDOW; }

void disable_window_transparency(void) { *LCDCONT |= LCD_TRWIN; }
void enable_window_transparency(void) { *LCDCONT &= ~LCD_TRWIN; }

void set_window_map_lo(void) { *LCDCONT &= ~LCD_WSEL; }
void set_window_map_hi(void) { *LCDCONT |= LCD_WSEL; }

void set_bg_map_lo(void) { *LCDCONT &= ~LCD_BGSEL; }
void set_bg_map_hi(void) { *LCDCONT |= LCD_BGSEL; }

void set_tiles_lo(void) { *LCDCONT |= LCD_BGTILES; }
void set_tiles_hi(void) { *LCDCONT &= ~LCD_BGTILES; }

void set_window_map(int i) {
  if (i) set_window_map_hi(); else set_window_map_lo();
}

void set_bg_map(int i) { if (i) set_bg_map_hi(); else set_bg_map_lo(); }

void set_tiles(int i) { if (i) set_tiles_hi(); else set_tiles_lo(); }

void set_bgpal(unsigned char c) { *BGPAL = c; }
void set_spritepal0(unsigned char c) { *SPRITEPAL0 = c; }
void set_spritepal1(unsigned char c) { *SPRITEPAL1 = c; }

void set_window_pos(int x, int y) {
 *WNDPOSX = x;
 *WNDPOSY = y;
}

void set_bg_pos(int x, int y) {
  *BGPOSX = x;
  *BGPOSY = y;
}

void wait_for_vblank() { while ((*LCDSTAT & 3) != 1); }
void wait_for_hblank() { while ((*LCDSTAT & 3) != 0); }
void wait_for_vblank_end() { while ((*LCDSTAT & 3) == 1); }
void wait_for_hblank_end() { while ((*LCDSTAT & 3) == 0); }

void set_tile_on_vblank(int x, int y, unsigned char t) {
  wait_for_vblank();
  LO_MAP[y][x] = t;
}

int log2(int s) {
  int i = 0;

  while (s) {
    s = s >> 1;
    if (s) i = i + 1;
  }

  return i;
}

int _div(int a, int b, int *r) {
  int i, q;

  int la = log2(a), lb = log2(b), ldiff = la - lb;

  for (i = 16 - ldiff, q = 0; i <= 16; i++) {
    int x = b << (16 - i), qx = 1 << (16 - i);

    if (a >= x) {
      q = q + qx;
      a = a - x;
    }
  }

  *r = a;

  return q;
}

int mod(int a, int b) {
  int r;
  _div(a, b, &r);
  return r;
}

int div(int a, int b) {
  int r;
  return _div(a, b, &r);
}

int mul(int a, int b) {
  int p = 0;

  while (a) {
    if (a & 1) p = p + b;
    a = a >> 1;
    b = b << 1;
  }

  return p;
}

#include "tiles.inc"

void set_tile(int id, int x, int y) {
  set_tile_on_vblank(x, y, id);
}

int scroll_x, scroll_y;
void set_scroll(int x, int y) {
  scroll_x = x;
  scroll_y = y;
  wait_for_vblank();
  set_bg_pos(x << 3, y << 3);
}

void clear(void) {
  int i, j;
  for (i = 0; i < 32; i++)
    for (j = 0; j < 32; j++)
      set_tile(' ', i, j);
}

int char_pos_x, char_pos_y, scrolling;
void gbputc(char c) {
  int y_scroll = 0;
  if (c == '\n') { char_pos_x=0; char_pos_y++; y_scroll = 1;}
  else {
    if (char_pos_y == 32) char_pos_y = 0;
    set_tile(c, char_pos_x, char_pos_y);
    if (!y_scroll) char_pos_x++;
    if (char_pos_x == 20) { char_pos_y++; char_pos_x = 0; y_scroll = 1; }
    if (char_pos_y == 18) scrolling = 1;
  }

  if (scrolling && y_scroll) {
    int i;
    set_scroll(0, (scroll_y + 1)&0x1f);
    for (i = 0; i < 32; i++)
      set_tile(' ', i, char_pos_y);
  }
}

char* gbitoa(char *s, int i) {
  if (i >= 10) { s = gbitoa(s, div(i,10)); i = mod(i,10); }
  
  *(s + 1) = 0;
  *s = i + '0';

  return s + 1;
}

void gbputs(const char *s) {
  while (*s) gbputc(*(s++));
}

void main(void);

void init(void) {
  int i, j;

  *LCDCONT = 0;
  disable_lcd();
  disable_window();
  set_bg_pos(0, 0);
  *IRQEN = 0;

  for (i = 0; i < 40; i++)
    (SPRITES + i)->x = (SPRITES + i)->y = 0;
  
  for (i = 0; i < 256; i++)
    for (j = 0; j < 16; j++)
      LO_TILES[i][j] = tiles[i][j];

  set_bg_map(0);
  set_tiles(0);

  set_bgpal(PAL_NORMAL);
  enable_bg();
  enable_lcd();

  main();

  for (;;);
}
 
#define N 300
#define SQRTN 32
int a[N];

void main(void) {
  int i, j;

  clear();
  set_scroll(0, 0);
  char_pos_x = char_pos_y = scrolling = 0;
  
  a[0] = a[1] = 0;
  for (i = 2; i < N; i++) a[i] = i;


  for (i = 0; i < SQRTN; i++)
    if (a[i])
      for (j = mul(i, i); j < N; j += i)
	a[j] = 0;

  j = 0;
  for (i = 0; i < N; i++) {
    if (a[i]) {
      char s[10];
      gbitoa(s, i);
      gbputs(s);
      if ((j++&3) == 3) gbputc('\n');
      else gbputc(' ');
    }
  }

  gbputs("@\n");

  for (;;);
}
