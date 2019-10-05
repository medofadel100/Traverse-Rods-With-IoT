#include "mbed.h"
#include "EthernetInterface.h"
#include "SDFileSystem.h"
#include <stdio.h>
#include <string.h>

#include "FTPClient.h"
#include "VS1002.h" 
#include "SSD1306.h"

#define MAC     "\x00\x08\xDC\x11\x34\x78"
#define IP      "192.168.0.20"
#define MASK    "255.255.255.0"
#define GATEWAY "192.168.0.1"

#define FTP_SERVER_IP "192.168.0.10"

#define _MAX_FNAME_LEN_   127
#define _FTP_UPDATE_TIME_  10


Serial uart(USBTX, USBRX);
AnalogIn   ain(A5);
//SDFileSystem sd(p5, p6, p7, p8, "sd"); // LPC1768 MBD2PMD
//SDFileSystem sd(P0_18, P0_17, P0_15, P0_16, "sd"); // Seeeduino Arch Pro SPI2SD
//SDFileSystem sd(PTE3, PTE1, PTE2, PTE4, "sd"); // K64F
//SDFileSystem sd(PB_3, PB_2, PB_1, PB_0, "sd"); // WIZwiki-W7500 

EthernetInterface eth;
InterruptIn K_VU(D3); // Create the interrupt receiver object on pin 26
InterruptIn K_VD(D7); // Create the interrupt receiver object on pin 26
VS1002 mp3(PB_3, PB_2, PB_1, PB_0,"sdc",D11, D12 ,D13, PC_12, PC_15, PC_14, PC_13);  //Setup Audio decoder. Name is VS1002 even though VS1053 is used.

FTPClient myFTP("/sdc");  // mountname in MySeeedStudioTFTv2

//Ticker ledTick;

/* Global Variables to store Status*/
int new_song_number=1;  //Variable to store the Song Number
int volume_set=-20;     //Variable to store the Volume
int previous_volume;    //Variable to store the volume when muted
bool pause=false;       //Variable to store the status of Pause button 
bool mute=false;        //Variable to store the status of mute button

int check=0;    //Capacitative touch generates interrupt on both press and release. This variable tracks this and updates only on press.
//char *song_name[9]={"Good Day","Leong","Sponsor","I'm So Sexy","My Life","Oh My god","Wonderful bar","Whale Hunting","Love"}; //Array of song names entered manually

uint32_t ftp_time_1s = 0;

char myfilelist[MAX_SS] = {0,};

/*void ledTickfunc()
{
    if(ftp_time_1s)
    {
        //printf("enter ftp_time_1s:%d\r\n", ftp_time_1s);
        if(ftp_time_1s++ > _FTP_UPDATE_TIME_) ftp_time_1s = 0;
    }
}*/
void Volume_Up() 
{
    volume_set+=3; // Volume Up
    if(volume_set>=0)
        volume_set=0;
}
 
void Volume_Down() 
{
    volume_set-=3;  //Volume Down
    if(volume_set<-55)
        volume_set=-55;
}
int main (void)
{
    *(volatile uint32_t *)(0x41001014) = 0x0060100;
    *(volatile uint32_t *)(0x41003000) = 0x10;
    *(volatile uint32_t *)(0x41003004) = 0x10;
    *(volatile uint32_t *)(0x41003008) = 0x10;
    *(volatile uint32_t *)(0x41003080) = 0x10;
    *(volatile uint32_t *)(0x41003098) = 0x10;
    
    unsigned int update_count_s=6;
    unsigned int update_count_ms=0;
    unsigned int ain_temp=0;
    //char* my_text = "GIF2015";
    /*char* tok = NULL;
    char* lasts = NULL;
    char filename[_MAX_FNAME_LEN_];
    FILE* fp;*/
    
    init();
    cls();
    //OLED_DrawBMP(0,0,128,8,(unsigned char *)GIF2015);
    //OLED_ShowStr(0,0,my_text,2);
    OLED_DrawBMP(0,0,128,8,(unsigned char *)wiznet);
    //LED_P23x32Str(0, 0, my_text);
//    Serial Interface eth;
    uart.baud(115200);
    uart.printf("Initializing\r\n");

//    EthernetInterface eth;
    uart.printf("Initializing Ethernet\r\n");

    //eth.init(); //Use DHCP
    eth.init((uint8_t*)MAC,IP,MASK,GATEWAY);  //IP,mask,Gateway
    uart.printf("Connecting\r\n");
    eth.connect();
    uart.printf("IP Address is %s\r\n", eth.getIPAddress());

//    Check File System
    uart.printf("Checking File System\r\n");
    DIR *d = opendir("/sdc/");
    if (d != NULL) {
        uart.printf("SD Card Present\r\n");
        closedir(d);
    } else {
        uart.printf("SD Card Root Directory Not Found\r\n");
    }

    //ledTick.attach(&ledTickfunc,2);
   /* UI Button setup */
    K_VU.fall(&Volume_Up);
    K_VU.mode(PullUp);
    K_VD.fall(&Volume_Down);
    K_VD.mode(PullUp);
    
    while(1)
    {
        update_count_ms++;
        if(update_count_ms>5000)
        {
            printf("update_count_ms count : %d\r\n", update_count_s);
            update_count_ms=0;
            update_count_s++;
        }
        //printf("ftp_time_1s:%d\r\n", ftp_time_1s);
        //if(ftp_time_1s == 0)
        if(update_count_s>=5)
        {
            //Configure the display driver
            update_count_s = 0;
            
            printf(" UPDATING MP3\r\n");
            printf("==================\n\r\n");
        
            if(myFTP.open("192.168.0.10", 21, "user", "pass"))
            {
                printf("Connect Success to FTPServer\r\n");
                printf("Connected to FTP Server\r\n");
                
                myFTP.getfile("1.mp3");
                myFTP.getfile("2.mp3");
                myFTP.getfile("3.mp3");
           
                printf("\n UPDATE DONE\n\r\n");
                myFTP.quit();
            }
            else
            {
                printf(" Can't connect to FTP Server\r\n");
                printf(" UPDATE FAIL\r\n");
            }
            printf("==================\n");
        }
        
        /*============================================================ 
         * MP3 Initialising 
         *==========================================================*/

        mp3._RST = 1; 
        mp3.cs_high();                                  //chip disabled 
        mp3.sci_initialise();                           //initialise MBED
        mp3.sci_write(0x00,(SM_SDINEW+SM_STREAM+SM_DIFF)); 
        mp3.sci_write(0x03, 0x9800); 
        mp3.sdi_initialise();

        ain_temp = ain.read_u16();
        //printf("normalized: %d \n\r", ain_temp);
        
        if (ain_temp>3000)
        {
            printf("enter song\r\n");
            mp3.play_song(new_song_number);
        }
    }    
}
