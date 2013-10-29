#ifndef chipKITDS1302_H_INCLUDED
#define chipKITDS1302_H_INCLUDED

/* RTC Chip register definitions */
#define SEC_WRITE    0x80
#define SEC_READ     0x81
#define MIN_WRITE    0x82
#define MIN_READ     0x83
#define HOUR_WRITE   0x84
#define HOUR_READ    0x85
#define DATE_WRITE   0x86
#define DATE_READ    0x87
#define MONTH_WRITE  0x88
#define MONTH_READ   0x89
#define YEAR_WRITE   0x8C
#define YEAR_READ    0x8D

#define REG_SEC		0
#define REG_MIN		1
#define REG_HOUR	2
#define REG_DATE	3
#define REG_MON		4
#define REG_DOW		5
#define REG_YEAR	6
#define REG_WP		7
#define REG_TCR		8

#define FORMAT_SHORT 1
#define FORMAT_LONG	2

#define MONDAY		1
#define TUESDAY		2
#define WEDNESDAY	3
#define THURSDAY	4
#define FRIDAY		5
#define SATURDAY	6
#define SUNDAY		7

//
struct DStime
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};
//

void DS1302WriteByte(uint8_t dat);
uint8_t DS1302ReadByte(void);
void DS1302WriteData(uint8_t addr,uint8_t dat);
uint8_t DS1302ReadData(uint8_t addr);
uint8_t DECToBCD(uint8_t dec);
uint8_t BCDToDEC(uint8_t bcd);

void DS1302Init(uint8_t sclk, uint8_t io, uint8_t ce, char * zone);
struct DStime DS1302GetTime(void);
void DS1302SetTime(struct DStime time);
void DS1302PrintTime();
void DS1302GetShortDateString(char * buffer, DStime time);
void DS1302GetShortTimeString(char * buffer, DStime time);
void DS1302GetShortDateTimeString(char * buffer, DStime time);
void DS1302GetHttpTimeString(char * buffer, DStime time);
void DS1302GetDOWStr(char *, DStime time, uint8_t format);
void DS1302GetMonthStr(char *, DStime time, uint8_t format);
#endif