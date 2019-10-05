#include "VS1002.h"
#include "mbed.h"

Serial pc(USBTX, USBRX);

/* ==================================================================
 * Constructor
 * =================================================================*/

VS1002::VS1002(PinName mmosi, PinName mmiso, PinName ssck, PinName ccs, const char *name, PinName mosi, PinName miso, PinName sck, PinName cs, PinName rst, PinName dreq, PinName dcs)
    : _DREQ(dreq), _RST(rst), _spi(mosi, miso, sck), _CS(cs), _DCS(dcs), _sd(mmosi, mmiso, ssck, ccs, name) {
    
    }    

/*===================================================================
 * Functions
 *==================================================================*/
 
void VS1002::cs_low(void)
{
    _CS = 0;                                
}
void VS1002::cs_high(void)
{
    _CS = 1;                                
}
void VS1002::dcs_low(void)
{
    _DCS = 0;
}
void VS1002::dcs_high(void)
{
    _DCS = 1;
}
void VS1002::sci_en(void)                    //SCI enable
{
    cs_high();
    dcs_high();
    cs_low();
}
void VS1002::sci_dis(void)                    //SCI disable
{
    cs_high();
}
void VS1002::sdi_en(void)                    //SDI enable
{
    dcs_high();
    cs_high();
    dcs_low();
}
void VS1002::sdi_dis(void)                    //SDI disable
{
    dcs_high();
}
void VS1002::reset(void)                    //hardware reset
{
    wait(0.01);
    _RST = 0;
    wait(0.01);
    _RST = 1;
    wait(0.10);
}
void VS1002::power_down(void)                //hardware and software reset
{
    cs_low();
    reset();
    sci_write(0x00, SM_PDOWN);
    wait(0.01);
    reset();
}
void VS1002::sci_initialise(void)
{
    _RST = 1;                                //no reset
    _spi.format(8,0);                        //spi 8bit interface, steady state low
    _spi.frequency(1000000);                //rising edge data record, freq. 1Mhz
    
    cs_low();
    for(int i=0; i<4; i++)
    {
    _spi.write(0xFF);                        //clock the chip a bit
    }
    cs_high();
    dcs_high();
    wait_us(5);
}
void VS1002::sdi_initialise(void)
{
    _spi.format(8,0);
    _spi.frequency(7000000);                //set to 7MHz
    
    cs_high();
    dcs_high();
}
void VS1002::sci_write(unsigned char address, unsigned short int data)
{
    sci_en();                         //enables SCI/disables SDI
    
    while(!_DREQ);                          //wait unitl data request is high
    _spi.write(0x02);                      //SCI write
    _spi.write(address);                   //register address
    _spi.write((data >> 8) & 0xFF);          //write out first half of data word
    _spi.write(data & 0xFF);              //write out second half of data word
    
    sci_dis();                            //enables SDI/disables SCI
    wait_us(5);
}
void VS1002::sdi_write(unsigned char datum)
{
    sdi_en();
    
    while(!_DREQ);
    _spi.write(datum);
    
    sci_dis();
}
unsigned short int VS1002::read(unsigned short int address)
{
    cs_low();                                //enables SCI/disables SDI
    
    while(!_DREQ);                            //wait unitl data request is high
    _spi.write(0x03);                        //SCI write
    _spi.write(address);                    //register address
    unsigned short int received = _spi.write(0x00);    //write out dummy byte
    received <<= 8;
    received += _spi.write(0x00);            //write out dummy byte
    
    cs_high();                                //enables SDI/disables SCI
    
    return received;                        //return received word
}
void VS1002::sine_test_activate(unsigned char wave)
{
    cs_high();                                //enables SDI/disables SCI
    
    while(!_DREQ);                            //wait unitl data request is high
    _spi.write(0x53);                        //SDI write
    _spi.write(0xEF);                        //SDI write
    _spi.write(0x6E);                        //SDI write
    _spi.write(wave);                        //SDI write
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte

    cs_low();                                //enables SCI/disables SDI
}
void VS1002::sine_test_deactivate(void)
{
    cs_high();
    
    while(!_DREQ);
    _spi.write(0x45);                        //SDI write
    _spi.write(0x78);                        //SDI write
    _spi.write(0x69);                        //SDI write
    _spi.write(0x74);                        //SDI write
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte
    _spi.write(0x00);                        //filler byte
}


void VS1002::volume(signed int left, signed int right)
{
    while(_DREQ == 0);
       
    unsigned short int _left = -left;       //convert the decibel values into a format
    unsigned short int _right = -right;     //readable by the chip cf. datasheet p.32 subsection 8.6.11
    _left *= 2;
    _right *= 2;
    unsigned short int attenuation = ((256 * _left) + _right);
    cs_low();
    sci_write(0x0B, attenuation);               //writeout these values
    cs_high();
}

void VS1002::play_song(int song_number)
{
    /*====== Song Select ======*/
    char str[16];        //folder where the songs are located
    sprintf(str,"/sdc/%d",song_number);   //appending song number to path of the file
    strcat(str,".mp3");                 //appending .mp3 to file name
    FILE *song;
    unsigned char array[512];           //array for reading data from file
    bool play_new=false;                // Variable to see if new_song has be assigned or not
    song = fopen(str, "r");    // Open the music file in read mode
   
    if(!song) 
    {
        new_song_number+=1;               // Goto Next song on completion of one song
        if(new_song_number==10)
            new_song_number=1;
    }
      while(!feof(song))
    {
            if(!pause)
           {
          
           fread(&array, 1, 512, song);           
           for(int i=0; i<512; i++)
           {
                sdi_write(array[i]);
           }
           volume(volume_set,volume_set);
           }
          if(new_song_number!=song_number)
           {
            play_new=true;
             break;
            }
             
             
          }
    
           fclose(song);                              //close the file

      if(!play_new)
      {
     new_song_number+=1;               // Goto Next song on completion of one song
     if(new_song_number==10)
      new_song_number=1; 
     play_new=false;                   
     }
}

