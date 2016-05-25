#define SER_DBG																				// serial debug messages

//- load library's --------------------------------------------------------------------------------------------------------
#include <AS.h>																				// ask sin framework
#include "register.h"																		// configuration sheet

#define OTA_DEVID_START  0x7ff0
#define OTA_SERIAL_START 0x7ff2
#define OTA_HMID_START   0x7ffc

void getDataFromAddressSection(uint8_t *buffer, uint16_t sectionAddress, uint8_t dataLen) {
  for (unsigned char i = 0; i < dataLen; i++) {
    buffer[i] = pgm_read_byte(sectionAddress + i);
  }
}

void readBootLoaderData () {
  uint8_t dev[2], ser[4];
  getDataFromAddressSection(dev,OTA_DEVID_START,2); // read device id
  getDataFromAddressSection(ser,OTA_SERIAL_START,4); // read start of serial
  // if device id equals and serial starts with same 4 byte
  // asume we have a valid ota bootloader and copy HMID and HMSerial
  if( dev[0]==0x00 && (dev[1]==0x02 || dev[1]==0x03 || dev[1]==0x0a) && memcmp(ser,HMSR,4)==0 ) {
	devIdnt[1] = dev[0];
	devIdnt[2] = dev[1];
    getDataFromAddressSection(HMID,OTA_HMID_START,3);
    getDataFromAddressSection(HMSR,OTA_SERIAL_START,10);
  }
}

//- arduino functions -----------------------------------------------------------------------------------------------------
void setup() {

	// - Hardware setup ---------------------------------------
	// - everything off ---------------------------------------

	EIMSK = 0;																				// disable external interrupts
	ADCSRA = 0;																				// ADC off
	power_all_disable();																	// and everything else
	
	DDRB = DDRC = DDRD = 0x00;																// everything as input
	PORTB = PORTC = PORTD = 0x00;															// pullup's off

	// todo: timer0 and SPI should enable internally
	power_timer0_enable();
	power_spi_enable();																		// enable only needed functions

	// enable only what is really needed

	#ifdef SER_DBG																			// some debug
		dbgStart();																			// serial setup
		dbg << F("HM_Relay_Board\n");	
		dbg << F(LIB_VERSION_STRING);
		_delay_ms (50);																		// ...and some information
	#endif

	readBootLoaderData();
	
	// - AskSin related ---------------------------------------
	hm.init();																				// init the asksin framework
	sei();																					// enable interrupts


	// - user related -----------------------------------------
	#ifdef SER_DBG
	  dbg << F("DEVID: ") << _HEX(&devIdnt[1],2) << F(", HMID: ") << _HEX(HMID,3) << F(", MAID: ") << _HEX(MAID,3) << F("\n");	// some debug
	  for( int i=0; i<10; ++i ) {
		  dbg << (char)HMSR[i];
	  }
	  dbg << F("\n");
	  dbg << F("Simulate HM_LC_SW") << devDef.cnlNbr << F("_SW\n\n");
	#endif
}

void loop() {
	// - AskSin related ---------------------------------------
	hm.poll();																				// poll the homematic main loop
	
	// - user related -----------------------------------------
	
}

uint8_t getSwitchPin(uint8_t channel) {
  switch(channel) {
    case 2: return 6; break;
    case 3: return 7; break;
    case 4: return 3; break;
  }
  return 5;
}

//- user functions --------------------------------------------------------------------------------------------------------
void initRly(uint8_t channel) {
// setting the relay pin as output, could be done also by pinMode(3, OUTPUT)
	#ifdef SER_DBG
		dbg << F("initRly: ") << channel << "\n";
	#endif
	uint8_t pin = getSwitchPin(channel);
	pinOutput(DDRD,pin);  // init the relay pins
	setPinLow(PORTD,pin); // set relay pin to ground
}
void switchRly(uint8_t channel, uint8_t status) {
// switching the relay, could be done also by digitalWrite(3,HIGH or LOW)
	#ifdef SER_DBG
		dbg << F("switchRly: ") << channel << ", " << status << "\n";
	#endif

	uint8_t pin = getSwitchPin(channel);
	// pinOutput(DDRD,pin);
        // check status and set relay pin accordingly																// init the relay pins
	if (status) {
           setPinHigh(PORTD,pin);
        }				
	else {
          setPinLow(PORTD,pin);
        }
}


//- predefined functions --------------------------------------------------------------------------------------------------
void serialEvent() {
	#if 0 //def SER_DBG
	
	static uint8_t i = 0;																	// it is a high byte next time
	while (Serial.available()) {
		uint8_t inChar = (uint8_t)Serial.read();											// read a byte
		if (inChar == '\n') {																// send to receive routine
			i = 0;
			hm.sn.active = 1;
		}
		
		if      ((inChar>96) && (inChar<103)) inChar-=87;									// a - f
		else if ((inChar>64) && (inChar<71))  inChar-=55;									// A - F
		else if ((inChar>47) && (inChar<58))  inChar-=48;									// 0 - 9
		else continue;
		
		if (i % 2 == 0) hm.sn.buf[i/2] = inChar << 4;										// high byte
		else hm.sn.buf[i/2] |= inChar;														// low byte
		
		i++;
	}
	#endif
}
