#include <WProgram.h>
#include <chipKITDS1302.h>

uint8_t _ce_pin;
uint8_t _data_pin;
uint8_t _sclk_pin;

static DStime dsTime;
static char * timeZone = "GMT";

void DS1302Init(uint8_t sclk, uint8_t io, uint8_t ce, char * zone) {
	_ce_pin = ce;
	_data_pin = io;
	_sclk_pin = sclk;
	timeZone = zone;
	
		/* Switch GPIO 0, 1 and 2 to output mode */
	pinMode(_sclk_pin, OUTPUT);
	pinMode(_data_pin, OUTPUT);
	pinMode(_ce_pin, OUTPUT);

	/* Set the SCLK, IO and CE pins to default (low) */
	digitalWrite(_sclk_pin, LOW);
	digitalWrite(_data_pin, LOW);
	digitalWrite(_ce_pin, LOW);

	/* Short delay to allow the I/O lines to settle. */
	delayMicroseconds(2);

}

void DS1302WriteByte(uint8_t dat) {
	pinMode(_data_pin, OUTPUT);
	uint8_t i;
	//set(TRST);
	for(i = 0;i < 8;i ++)
	{
		if((dat & 0x01) == 0x01)
			digitalWrite(_data_pin, HIGH);
		else
			digitalWrite(_data_pin, LOW);
		digitalWrite(_sclk_pin, HIGH);
		digitalWrite(_sclk_pin, LOW);
		dat >>= 1;	
	}
}

uint8_t DS1302ReadByte(void) {
	pinMode(_data_pin, INPUT);
	digitalWrite(_data_pin, LOW);       // turn off pullup resistors
	uint8_t i,dat = 0;
	//set(TRST);

	uint8_t value = 0;
	uint8_t currentBit = 0;

	for (int i = 0; i < 8; ++i) {
		currentBit = digitalRead(_data_pin);
		value |= (currentBit << i);
		digitalWrite(_sclk_pin, HIGH);
		delayMicroseconds(1);
		digitalWrite(_sclk_pin, LOW);
	}
	return value;
}

void DS1302WriteData(uint8_t addr, uint8_t dat) {
	digitalWrite(_ce_pin, LOW);
	digitalWrite(_sclk_pin, LOW);
	digitalWrite(_ce_pin, HIGH);
	DS1302WriteByte(addr);
	DS1302WriteByte(dat);
	digitalWrite(_sclk_pin, HIGH);
	digitalWrite(_ce_pin, LOW);
}


uint8_t DS1302ReadData(uint8_t addr) {
	uint8_t dat;
	digitalWrite(_ce_pin, LOW);
	digitalWrite(_sclk_pin, LOW);
	digitalWrite(_ce_pin, HIGH);
	DS1302WriteByte(addr);
	dat = DS1302ReadByte();
	digitalWrite(_sclk_pin, HIGH);
	digitalWrite(_ce_pin, LOW);
	return(dat);
}


uint8_t DECToBCD(uint8_t dec) {
	uint8_t bcd;
	bcd = 0;
	while(dec >= 10)
	{              
		dec -= 10;                         
		bcd++;
	} 
	bcd <<= 4;
	bcd |= dec;
	return bcd;
}


uint8_t	decode(uint8_t value) {
	uint8_t decoded = value & 127;
	decoded = (decoded & 0x0F) + 10 * ((decoded & 0x70) >> 4);
	return decoded;
}


uint8_t BCDToDEC(uint8_t bcd) {
	uint8_t dec;
	dec = bcd & 0x0F;
	bcd = bcd & 0x70;
	dec += bcd >> 1;
	dec += bcd >> 3;
	return dec;
}


struct DStime DS1302GetTime(void) {
	DS1302WriteData(0x8E,0);//close write protect
	dsTime.second = decode(DS1302ReadData(0x81));//get second
	dsTime.minute = decode(DS1302ReadData(0x83));//get minute
	dsTime.hour = BCDToDEC(DS1302ReadData(0x85));//get hour
	dsTime.day = BCDToDEC(DS1302ReadData(0x87));//get week
	dsTime.month = BCDToDEC(DS1302ReadData(0x89));//get month 
	dsTime.week = BCDToDEC(DS1302ReadData(0x8B));//get day
	dsTime.year = BCDToDEC(DS1302ReadData(0x8D));//get year
	DS1302WriteData(0x8E,0x80);//open write protect
	return(dsTime);
}


void DS1302SetTime(struct DStime time) {
	DS1302WriteData(0x8E,0);//close write protect
	DS1302WriteData(0x80,DECToBCD(time.second));//set second
	DS1302WriteData(0x82,DECToBCD(time.minute));//set minute
	DS1302WriteData(0x84,DECToBCD(time.hour));//set hour
	DS1302WriteData(0x86,DECToBCD(time.day));//set week
	DS1302WriteData(0x88,DECToBCD(time.month));//set month 
	DS1302WriteData(0x8A,DECToBCD(time.week));//set day
	DS1302WriteData(0x8C,DECToBCD(time.year));//set year
	DS1302WriteData(0x8E,0x80);//open write protect
}

void DS1302GetShortDateString(char * buffer, DStime dsTime) {
	sprintf(buffer, "%02d/%02d/%02d", dsTime.month, dsTime.day, dsTime.year);
}

void DS1302GetShortTimeString(char * buffer, DStime dsTime) {
	sprintf(buffer, "%02d:%02d:%02d", dsTime.hour, dsTime.minute, dsTime.second);
}

void DS1302GetShortDateTimeString(char * buffer, DStime dsTime) {
	sprintf(buffer, "%02d/%02d/%02d %02d:%02d:%02d", dsTime.month, dsTime.day, dsTime.year, dsTime.hour, dsTime.minute, dsTime.second);
}

/* Return string of format "Mon, 1 Jul 2013 09:02:10 GMT"	*/
void DS1302GetHttpTimeString(char * buffer, DStime dsTime) {
	char monBuff[10];
	char dowBuff[10];
	DS1302GetDOWStr(dowBuff, dsTime, FORMAT_SHORT);
	DS1302GetMonthStr(monBuff, dsTime, FORMAT_SHORT);
	sprintf(buffer, "%s, %d %s %d %02d:%02d:%02d %s", 
			dowBuff,
			dsTime.day,
			monBuff,
			(2000 + dsTime.year),
			dsTime.hour, dsTime.minute, dsTime.second,
			timeZone);	
}

void DS1302PrintTime() {
	static int year,month,day,hour,minute,second;
	dsTime = DS1302GetTime();

	second = dsTime.second;
	minute = dsTime.minute;
	hour = dsTime.hour;
	day = dsTime.day;
	month = dsTime.month;
	year = dsTime.year;   

	Serial.print(String(hour, DEC));Serial.print(":");
	Serial.print(String(minute, DEC));Serial.print(":");
	Serial.print(String(second, DEC));Serial.print(" ");
	Serial.print(timeZone);Serial.print(" ");
	Serial.print(String(day, DEC));Serial.print("-");
	Serial.print(String(month, DEC));Serial.print("-");
	Serial.println(String(year, DEC));
}

 void DS1302GetDOWStr(char* buf, DStime t, uint8_t format) {
	switch (t.week) {
		case MONDAY:
			sprintf(buf, "Monday");
			break;
		case TUESDAY:
			sprintf(buf, "Tuesday");
			break;
		case WEDNESDAY:
			sprintf(buf, "Wednesday");
			break;
		case THURSDAY:
			sprintf(buf, "Thursday");
			break;
		case FRIDAY:
			sprintf(buf, "Friday");
			break;
		case SATURDAY:
			sprintf(buf, "Saturday");
			break;
		case SUNDAY:
			sprintf(buf, "Sunday");
			break;
	}   
	if (format==FORMAT_SHORT)
		buf[3] = '\0';		
//	return buf;
}

void DS1302GetMonthStr(char* buf, DStime t, uint8_t format) {
	switch (t.month) {
		case 1:
			sprintf(buf, "January");
			break;
		case 2:
			sprintf(buf, "February");
			break;
		case 3:
			sprintf(buf, "March");
			break;
		case 4:
			sprintf(buf, "April");
			break;
		case 5:
			sprintf(buf, "May");
			break;
		case 6:
			sprintf(buf, "June");
			break;
		case 7:
			sprintf(buf, "July");
			break;
		case 8:
			sprintf(buf, "August");
			break;
		case 9:
			sprintf(buf, "September");
			break;
		case 10:
			sprintf(buf, "October");
			break;
		case 11:
			sprintf(buf, "November");
			break;
		case 12:
			sprintf(buf, "December");
			break;
	}
	if (format==FORMAT_SHORT)
		buf[3] = '\0';
//	return output;
}