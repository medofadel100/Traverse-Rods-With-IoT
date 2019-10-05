#include "data.h"
#define addr    (0x78)

#if defined(TARGET_WIZwiki_W7500)
#define SDA                  PA_10
#define SCL                  PA_9
#endif

I2C i2c(SDA, SCL);

void send_cmd(uint8_t cmd){
     char c[2] ={0x00,cmd};
     
       i2c.write(addr,c,2,1);
     
}

void send_data(uint8_t data){
    char c[2] = {0x40,data};
    i2c.write(addr,c,2,1);
    
}

void init(void)
{
    wait_ms(5); // TBD
       
    send_cmd(0xae); // display off
    send_cmd(0xd5); // display divide ratio/osc. freq. ratio
    send_cmd(0x80);
    send_cmd(0xa8); // multiplex ation mode: 63
    send_cmd(0x3f);
    send_cmd(0xd3); // set display offset
    send_cmd(0x00);
    send_cmd(0x40); // set display start line
    send_cmd(0x8d); // set display offset
    send_cmd(0x14);
    send_cmd(0xa1); // segment remap
    send_cmd(0xc8); // set COM output scan direction
    send_cmd(0xda); // common pads hardware: alternative
    send_cmd(0x12);
    send_cmd(0x81); // contrast control
    send_cmd(0xcf);
    send_cmd(0xd9); // set pre-charge period
    send_cmd(0xf1);
    send_cmd(0xdb); // VCOM deselect level mode
    send_cmd(0x40); // set vcomh = 0.83 * VCC
    send_cmd(0xa4); // set entire display on/off
    send_cmd(0xa6); // set normal display
    send_cmd(0xaf); // set display on
}

// set position (x, 8*y)
void locate(int x, int y){
    send_cmd(0xb0+y);
    send_cmd(((x&0xf0)>>4)|0x10);
    send_cmd((x&0x0f)|0x01);
}

void cls(void){
    int x, y;
    for(y = 0; y < 8; y++){
        locate(0, y);
        for(x = 0; x < 128; x++) send_data(0x00);
    }
}



 void OLED_ShowStr(unsigned char x, unsigned char y, char ch[], unsigned char TextSize)
{
    unsigned char c = 0,i = 0,j = 0;
    switch(TextSize)
    {
        case 1:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 126)
                {
                    x = 0;
                    y++;
                }
                locate(x,y);
                for(i=0;i<6;i++)
                    send_data(F6x8[c][i]);
                x += 6;
                j++;
            }
        }break;
        case 2:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 120)
                {
                    x = 0;
                    y++;
                }
                locate(x,y);
                for(i=0;i<8;i++)
                    send_data(F8X16[c*16+i]);
                locate(x,y+1);
                for(i=0;i<8;i++)
                    send_data(F8X16[c*16+i+8]);
                x += 8;
                j++;
            }
        }break;
    }
}
void OLED_DrawBMP(unsigned char x0,
                           unsigned char y0,unsigned char x1,
                           unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

  if(y1%8==0)
        y = y1/8;
  else
        y = y1/8 + 1;
    for(y=y0;y<y1;y++)
    {
        locate(x0,y);
    for(x=x0;x<x1;x++)
        {
            send_data(BMP[j++]);
        }
    }
}
//==========================================================//
// Prints a display big number (96 bytes) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15)
// and 8 ROWS (0-7).
void printBigNumber(unsigned char s, int x, int y)
{
    locate(x,y);
    int salto=0;
    for(int i=0; i<96; i++) {
        if(s == ' ') {
            send_data(0);
        } else
            send_data(bigNumbers[s-0x30][i]);

        if(salto == 23) {
            salto = 0;
            x++;
            locate(x,y);
        } else {
            salto++;
        }
    }
}
void printBigTime(char *s)
{

    int y=0;
    int lon = strlen(s);
    if(lon == 6) {
        y = 0;
    } else if (lon == 5) {
        y = 3;
    } else if (lon == 4) {
        y = 6;
    }
      else if(lon == 3) {
        y = 9;
    } else if (lon == 2) {
        y = 12;
    } else if (lon == 1) {
        y = 15;
    }

    int x = 2;
    while(*s) {
        printBigNumber(*s, x, y);

        y+=3;
        x=2;
        locate(x,y);
        *s++;
    }
}
void LED_P23x32Str(unsigned char x, unsigned char y, char ch[])
{
  unsigned char c = 0, i = 0, j = 0, k = 0;
  while (ch[j] != '\0')
  {
    c = ch[j] - '0';
    if (x > 120) {
      x = 0;
      y++;
    }

    for (k = 0; k < 4; k++) {
      locate(x, y + k);
      for (i = 0; i < 23; i++) {
        send_data(F23x32[c * 23 * 4 + k * 23 + i]);
      }
    }
    x += 25;
    j++;
  }
}


