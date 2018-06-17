
#include "mappage_led.h"


//for  the moment the matrix is fixed in size: 256 leds
#define NUM_LED 256
 //char map_led[NUM_LED]=

#define NUM_PANNEL_X  4
#define NUM_PANNEL_Y  1


uint16_t get_map_num_led(pixel_position pos)
{
  uint16_t num_led = 0;
  while(pos.y > 7)
  {
    pos.y -= 8;
    num_led += NUM_PANNEL_X * 64;
  }
  
  while(pos.x > 7)
  {
    pos.x -= 8;
    num_led += 64; 
  }

  if((pos.y % 2) == 1)//impair
  {
    if(num_led < 150)
      num_led = num_led + (pos.y + 1)*8 - pos.x - 1;
    else
      num_led = num_led + pos.x + (pos.y * 8);
  }
  else
  {
      num_led = num_led + pos.x + (pos.y * 8);
  }
  return num_led;
}


//get position real led
void get_map_led(pixel_position* pos)
{
  return;
}

