/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.07.2015
Descr: Implement software SPI. To improve speed, GPIO16 is not supported(see Digital.cpp)
*/
#include "SPISoft.h"

#define SPEED 0 /* You gain ~0.7 kBps (more for larger data chunks)*/
#define SIZE 1 /* You gain ~ 400B from the total 32K of cache RAM */
#define SPEED_VS_SIZE SIZE /* Your choice here, I choose SIZE */

#define GP_IN(pin)	((GPIO_REG_READ(GPIO_IN_ADDRESS)>>(pin)) & 1)
#define GP_OUT(pin, val) GPIO_REG_WRITE(((((val) != LOW) ? \
							GPIO_OUT_W1TS_ADDRESS : \
							GPIO_OUT_W1TC_ADDRESS)), ((uint16_t)1<<(pin)))
#define CPHA1
#define CPOL1

#ifdef CPHA1

#ifdef CPOL1
#define SCK_PULSE	GP_OUT(mCLK, LOW); \
					delayMicroseconds(1); \
					GP_OUT(mCLK, HIGH); \
					delayMicroseconds(1);

#define SCK_PULSEfast	GP_OUT(mCLK, LOW); \
					fastDelay(m_delay); \
					GP_OUT(mCLK, HIGH); \
					fastDelay(m_delay);

#endif
#ifdef CPOL0
#define SCK_PULSE	GP_OUT(mCLK, HIGH); \
					fastDelay(m_delay); \
					GP_OUT(mCLK, LOW); \
					fastDelay(m_delay);
#endif
#endif
#ifdef CPHA0
#ifdef CPOL1
#define SCK_PULSE	fastDelay(m_delay); \
					GP_OUT(mCLK, LOW); \
					fastDelay(m_delay); \
					GP_OUT(mCLK, HIGH);
#endif
#ifdef CPOL0
#define SCK_PULSE	fastDelay(m_delay); \
					GP_OUT(mCLK, HIGH); \
					fastDelay(m_delay); \
					GP_OUT(mCLK, LOW);
#endif
#endif
static inline void IRAM_ATTR fastDelay(unsigned d)
{
	while(d) --d;
}

void SPISoft::begin()
{
	if(16 == mMISO || 16 == mMOSI || 16 == mCLK || 16 == mCS )
	{
		/*To be able to use fast/simple GPIO read/write GPIO16 is not supported*/
		debugf("SPISoft: GPIO 16 not supported\n");
		return;
	}

	pinMode(mCLK, OUTPUT);
#ifdef CPOL0
	digitalWrite(mCLK, LOW);
#endif
#ifdef CPOL1
	digitalWrite(mCLK, HIGH);
#endif

	pinMode(mMISO, INPUT);
	digitalWrite(mMISO, HIGH);

	pinMode(mMOSI, OUTPUT);

	pinMode(mCS, OUTPUT);
	digitalWrite(mCS, HIGH);
}

void SPISoft::transfer(uint8_t* buffer, uint32_t size)
{
	uint8_t d;
	uint8_t r;
	do {
		d = *buffer; r=0;

#ifdef CPHA1
		GP_OUT(mMOSI, d & 0x80);	/* bit7 */
		SCK_PULSE
#endif
		r = 		 GP_IN(mMISO); //bit 7
		GP_OUT(mMOSI, d & 0x40);	/* bit6 */
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x20);	/* bit5 */
		r = r << 1 | GP_IN(mMISO); //bit 6
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x10);	/* bit4 */
		r = r << 1 | GP_IN(mMISO); //bit 5
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x08);	/* bit3 */
		r = r << 1 | GP_IN(mMISO); //bit 4
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x04);	/* bit2 */
		r = r << 1 | GP_IN(mMISO); //bit 3
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x02);	/* bit1 */
		r = r << 1 | GP_IN(mMISO); //bit 2
		SCK_PULSE

		GP_OUT(mMOSI, d & 0x01);	/* bit0 */
		r = r << 1 | GP_IN(mMISO); //bit 1
		SCK_PULSE
		r = r << 1 | GP_IN(mMISO); //bit 0
#ifdef CPHA0
		SCK_PULSE
#endif
		delayMicroseconds(m_byte_delay);
		*buffer++ = r;
	} while (--size);
}

