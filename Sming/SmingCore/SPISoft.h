/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
*/
#ifndef _SPI_SOFT_
#define _SPI_SOFT_

#include "SPIBase.h"
#include "SPISettings.h"
#include "Clock.h"


class SPISoft: public SPIBase {

public:

	SPISoft(uint16_t miso, uint16_t mosi, uint16_t sck, uint16_t cs, uint8_t delay, uint16_t byte_delay) {
		mMISO = miso;
		mMOSI = mosi;
		mCLK = sck;
		mCS = cs;
		m_delay = delay;
		m_byte_delay = byte_delay;
	}

	virtual ~SPISoft() {};

	/*
	 *  begin(): Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	 */
	virtual void begin();	//setup pins

	/*
	 * end(): Disables the SPI bus (leaving pin modes unchanged).
	 */
	virtual void end() {};

	/*
	 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
	 */
	virtual void beginTransaction(SPISettings mySettings) {	digitalWrite(mCS, LOW);};

	/*
	 * endTransaction(): Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	virtual void endTransaction() {	digitalWrite(mCS, HIGH);};

	/*
	 * transfer(), transfer16()
	 *
	 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16). In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
	 *
	 * 		receivedVal = SPI.transfer(val)
	 * 		receivedVal16 = SPI.transfer16(val16)
	 * 		SPI.transfer(buffer, size)
	 */
	void transfer(uint8 * buffer, size_t size);
	unsigned char transfer(unsigned char val) {transfer(&val, 1); return val;};
	unsigned short transfer16(unsigned short val) {transfer((uint8 *)&val, 2); return val;};


	/**
	\brief Set microsecond delay for the SCK signal. Impacts SPI speed
	*/
	inline void setDelay(uint8_t dly) {m_delay = dly;}

private:
	uint16_t mMISO, mMOSI, mCLK, mCS;
	SPISettings mSPISettings;
	uint8_t m_delay;
	uint16_t m_byte_delay;
};

#endif /*_SPI_SOFT_*/
