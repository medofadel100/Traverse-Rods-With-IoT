#ifndef VS1002_H
#define VS1002_H

#include "mbed.h"
#include "SDFileSystem.h"
#include "string"
#include "string.h"


//SCI_MODE register bits as of p.26 of the datasheet
#define SM_DIFF            0x0001
#define SM_SETTOZERO    0x0002
#define SM_RESET        0x0004
#define SM_OUTOFWAV        0x0008
#define SM_PDOWN        0x0010
#define SM_TESTS        0x0020
#define    SM_STREAM        0x0040
#define SM_PLUSV        0x0080
#define    SM_DACT            0x0100
#define    SM_SDIORD        0x0200
#define    SM_SDISHARE        0x0400
#define    SM_SDINEW        0x0800
#define    SM_ADPCM        0x1000
#define    SM_ADPCM_HP        0x2000

extern int new_song_number;
extern int volume_set;
extern bool pause;
extern bool mute;
extern char * song_name[9];


class VS1002  {

public:

    VS1002(PinName mmosi, PinName mmiso, PinName ssck, PinName ccs, const char *name, PinName mosi, PinName miso, PinName sck, PinName cs, PinName rst, PinName dreq, PinName dcs);
     
    void cs_low(void); 
    void cs_high(void);
    void dcs_low(void);
    void dcs_high(void);
    void sci_en(void);
    void sci_dis(void);
    void sdi_en(void);
    void sdi_dis(void);
    
    void sci_initialise(void);
    void sdi_initialise(void);
    void reset(void);
    void power_down(void);
 
    void sci_write(unsigned char, unsigned short int);
    void sdi_write(unsigned char);
    unsigned short int read(unsigned short int);
    void sine_test_activate(unsigned char);
    void volume(signed int,signed int);
    void sine_test_deactivate(void);
    void play_song(int);
    
    int num_of_files;
    
    DigitalIn _DREQ;
    DigitalOut _RST;
    
protected:

    SPI _spi;
    DigitalOut _CS;
    DigitalOut _DCS;
    SDFileSystem _sd;

};
#endif