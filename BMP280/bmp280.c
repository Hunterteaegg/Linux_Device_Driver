#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;

int main()
{
    int fd;
    int bit_per_word = 8;
    u8 mode;
    u8 rx_buff[1] = {0};
    u8 tx_buff[1] = {0};
    
    fd = open("/dev/spidev0.0",O_RDWR);
    if(fd < 0)
    {
        printf("open /dev/spidev0.0 failed.\n");
        exit(-1);
    }
    printf("open /dev/spidev0.0 successfully.\n");


    int res;
    res = ioctl(fd,SPI_IOC_RD_MODE,&mode);
    if(res < 0)
    {
        printf("setting SPI rx Mode failed.\n");
        exit(-1);
    }
    res = ioctl(fd,SPI_IOC_WR_MODE,&mode);
    if(res < 0)
    {
        printf("setting SPI tx Mode failed.\n");
        exit(-1);
    }
    res = ioctl(fd,SPI_IOC_RD_BITS_PER_WORD,&bit_per_word);
    res = ioctl(fd,SPI_IOC_WR_BITS_PER_WORD,&bit_per_word);

    printf("setting SPI Mode successfully.\n");
    printf("SPI Mode are tx:%d, rx:%d.\n",mode,mode);

    tx_buff[0] = (0x80 | 0xD0);
    /*
    struct spi_ioc_transfer msg;
    memset(&msg, 0, sizeof(msg));

    msg.rx_buf = (unsigned long long)rx_buff;
    msg.tx_buf = (unsigned long long)tx_buff;
    msg.len = 1;
    msg.bits_per_word = 8;
    msg.speed_hz = 80000;
    */
    
    volatile struct spi_ioc_transfer msg[2];
    memset((void*)msg, 0, sizeof(msg));

    msg[0].rx_buf = (unsigned long long)rx_buff;
    msg[0].tx_buf = (unsigned long long)tx_buff; 
    msg[0].len =1;
    msg[0].bits_per_word = 8;
    msg[0].speed_hz = 8 * 1000 * 1000;

    msg[1].rx_buf = (unsigned long long)rx_buff;
    msg[1].tx_buf = (unsigned long long)tx_buff; 
    msg[1].len =1;
    msg[1].bits_per_word = 8;
    msg[1].speed_hz = 8 * 1000 * 1000;

    res = ioctl(fd,SPI_IOC_MESSAGE(2),msg);
    if(res < 0)
    {
        printf("SPI read failed.\n");
        exit(-1);
    }
    printf("SPI read successfully.\n");
    printf("The value of ID is 0x%2X.\n",rx_buff[0]);
    
    close(fd);

    return 0;
}
