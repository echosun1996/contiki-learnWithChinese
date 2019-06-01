#ifndef __DS18B20_H
#define	__DS18B20_H

#define DQ_PORT   GPIO_B_BASE
#define DQ_BIT    2

#define DS18B20_DQ_OUT(a)   if (a)	\
				GPIO_WRITE_PIN(DQ_PORT, DQ_BIT, DQ_BIT);\
		            else		\
		                GPIO_WRITE_PIN(DQ_PORT, DQ_BIT, 0)

#define DS18B20_DQ_IN      GPIO_READ_PIN(DQ_PORT,DQ_BIT)

void DS18B20_IO_OUT(void);
void DS18B20_IO_IN(void);
void DS18B20_Rst(void);
unsigned char DS18B20_Check(void);
unsigned char DS18B20_Read_Bit(void);     // read one bit
unsigned char DS18B20_Read_Byte(void);    // read one byte
void DS18B20_Write_Byte(unsigned char dat);
void DS18B20_Start(void);// ds1820 start convert 	 
unsigned char DS18B20_Init(void);
short DS18B20_Get_Temp(void);
void ds18b20_read(unsigned char * pChBuf);

#endif

