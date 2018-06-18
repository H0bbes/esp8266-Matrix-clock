#include <stdint.h>

struct pixel_position
{
  uint16_t x;
  uint16_t y;
};

struct pixel_color
{
  char red;
  char green;
  char blue;
};


typedef struct pixel_position pixel_position;

void test();
uint16_t get_map_num_led(pixel_position pos);
