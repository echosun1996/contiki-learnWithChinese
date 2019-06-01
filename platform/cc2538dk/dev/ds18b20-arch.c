#include "reg.h"
#include "dev/ds18b20-arch.h"
#include "dev/gpio.h"
#include "clock.h"

void delay_us(unsigned int time)
{
  clock_delay_usec(time);
}

void DS18B20_IO_OUT(void)
{
  GPIO_SET_OUTPUT(DQ_PORT,DQ_BIT); 
}

void DS18B20_IO_IN(void)
{
  GPIO_SET_INPUT(DQ_PORT,DQ_BIT);
}

//复位DS18B20
void DS18B20_Rst(void)	   
{                 
    DS18B20_IO_OUT(); 
    DS18B20_DQ_OUT(0);
    delay_us(750);    
    DS18B20_DQ_OUT(1);
    delay_us(15);     
}
//等待DS18B20的回应
//返回1:未检测到DS18B20的存在
//返回0:存在
unsigned char DS18B20_Check(void) 	   
{   
    unsigned char retry=0;
    DS18B20_IO_IN();	 
    while(DS18B20_DQ_IN&&retry<200)
   {
      retry++;
      delay_us(1);
   };	 
    if(retry>=200)return 1;
    else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
   {
      retry++;
      delay_us(1);
    };
    if(retry>=240)return 1;	    
    return 0;
}
//从DS18B20读取一个位
//返回值：1/0
unsigned char DS18B20_Read_Bit(void)
{
    unsigned char data;
    DS18B20_IO_OUT();
    DS18B20_DQ_OUT(0);
 
    delay_us(2);
    DS18B20_DQ_OUT(1);

    DS18B20_IO_IN();
    delay_us(12);
    if(DS18B20_DQ_IN)data=1;
    else data=0;	 
    delay_us(50);           
    return data;
}
//从DS18B20读取一个字节
//返回值：读到的数据
unsigned char DS18B20_Read_Byte(void)   
{        
    unsigned char i,j,dat;
    dat=0;
    for (i=1;i<=8;i++) 
   {
      j=DS18B20_Read_Bit();
      dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}
//写一个字节到DS18B20
//dat：要写入的字节
void DS18B20_Write_Byte(unsigned char dat)     
 {             
    unsigned char j;
    unsigned char testb;
    DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
   {
       testb=dat&0x01;
       dat=dat>>1;
       if (testb) 
       {
           DS18B20_DQ_OUT(0);
           delay_us(2); 
           DS18B20_DQ_OUT(1);                           
           delay_us(60);             
        }
       else 
      {
           DS18B20_DQ_OUT(0);
           delay_us(60); 
           DS18B20_DQ_OUT(1);            
           delay_us(2);                          
       }
    }
}
//ds1820 start convert
void DS18B20_Start(void)
{   						               
    DS18B20_Rst();	   
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//初始化DS18B20的IO口 DQ 同时检测DS的存在
//返回1:不存在
//返回0:存在    	 
unsigned char DS18B20_Init(void)
{
    GPIO_WRITE_PIN(DQ_PORT, DQ_BIT, DQ_BIT); 

    DS18B20_Rst();

    return DS18B20_Check();
} 
//从ds18b20得到温度值
//返回值：温度值 （-550~1250） 
short DS18B20_Get_Temp(void)
{
    unsigned char temp;
    unsigned char TL,TH;
    short tem;
    DS18B20_Start ();          // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// 	    
    TL=DS18B20_Read_Byte(); // LSB   
    TH=DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;//温度为负  
    }else temp=1;//温度为正	  	  
    tem=TH; //获得高八位
    tem<<=8;    
    tem+=TL;//获得底八位
    tem=(float)tem*0.0625;//转换     
    if(temp)return tem; //返回温度值
    else return -tem;    
} 
void ds18b20_read(unsigned char * pChBuf)
{
  //uint8_t pChBuf[8];
  unsigned char num;
  short temp=DS18B20_Get_Temp();
  num=temp*10000; 
  //printf("raw temp value:%d \n",num);
  if(temp<0)  
  {
   	pChBuf[0]='-';
  }
  else pChBuf[0]='+';   
  if(temp/100==0){
     	pChBuf[1]=' ';
	}
  else {
   	pChBuf[1]=temp/100+0x30; 
	}
  if((temp/10%10==0)&&(temp/100==0))
   	pChBuf[2]=' ';
  else  pChBuf[2]=temp/10%10+0x30;

    pChBuf[3]=temp%10+0x30;
    pChBuf[4]='.';
    pChBuf[5]=num/1000+0x30;
    pChBuf[6]='C';
    pChBuf[7]='\0';
    /*tmp=(temp/10)*10 + temp%10;


  if(tmp>20){
	leds_on(LEDS_RED);
  }else{
	leds_off(LEDS_RED);
  }


  PRINTF("my tmp: %d \n",tmp);*/

  //return *pChBuf;
  
}
