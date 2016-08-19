//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin config key function --------------------------------------------------------------------------------------------
//- with a lot of support from martin876 at FHEM forum
//- -----------------------------------------------------------------------------------------------------------------------

// #define CB_DBG
#include "ConfButton.h"
#include "AS.h"

waitTimer btnTmr;																			// button timer functionality

// public:		//---------------------------------------------------------------------------------------------------------
void CB::config(uint8_t mode) {
	scn = mode;
}

// private:		//---------------------------------------------------------------------------------------------------------
CB::CB() {
	#ifdef CB_DBG																			// only if ee debug is set
	dbgStart();																				// serial setup
	dbg << F("CB.\n");																		// ...and some information
	#endif
}
//void CB::init(AS *ptrMain) {
//	pHM = ptrMain;
//}
void CB::poll(void) {
	#define detectLong      3000
	#define repeatedLong    300
	#define timeoutDouble   1000
	
	if (!scn) return;																		// mode not set, nothing to do
	
	// 0 for button is pressed, 1 for released, 2 for falling and 3 for rising edge
	btn = checkConfKey();																	// check input pin

	if (btn == 2) {																			// button was just pressed
		//dbg << "armed \n";
		btnTmr.set(detectLong);																// set timer to detect a long
		hm.pw.stayAwake(detectLong+500);													// stay awake to check button status
		armFlg = 1;																			// set it armed
		return;
	}

	if (!armFlg) return;																	// nothing to do any more
	
	
	// check button status
	if (btn == 3) {									// button was just released, keyShortSingle, keyShortDouble, keyLongRelease
		//dbg << "3 lstLng:" << lstLng << " dblLng:" << dblLng << " lngRpt:" << lngRpt << " lstSht:" << lstSht << '\n';

		btnTmr.set(timeoutDouble);															// set timer to clear the repeated flags
		hm.pw.stayAwake(timeoutDouble+500);													// stay awake to check button status
		
		if       ((lstLng) && (lngRpt)) {			// keyLongRelease
			outSignal(5);

		} else if (lstLng) {						// no action, only remember for a double
			dblLng = 1;																		// waiting for a key long double

		} else if (lstSht) {						// keyShortDouble
			lstSht = 0;	
			outSignal(2);	

		} else if ((!lstLng) && (!dblLng)) {		// keyShortSingle
			lstSht = 1;																		// waiting for a key short double
			outSignal(1);

		}
		lstLng = lngRpt = 0;																// some cleanup
		
	} else if ((btn == 0) && (btnTmr.done() )) {	// button is still pressed, but timed out, seems to be a long
		//dbg << "0 lstLng:" << lstLng << " dblLng:" << dblLng << " lngRpt:" << lngRpt << " lstSht:" << lstSht << '\n';

		hm.pw.stayAwake(detectLong+500);													// stay awake to check button status

		if (lstLng) {								// keyLongRepeat
			btnTmr.set(repeatedLong);														// set timer to detect a repeated long
			lngRpt = 1;
			outSignal(4);																	// last key state was a long, now it is a repeated long

		} else if (dblLng) {						// keyLongDouble
			btnTmr.set(detectLong);															// set timer to detect next long
			outSignal(6);																	// double flag is set, means was the second time for a long													

		} else {									// keyLongSingle
			btnTmr.set(detectLong);															// set timer to detect a repeated long
			lstLng = 1;																		// remember last was long
			outSignal(3);																	// first time detect a long

		}
		
		
	} else if ((btn == 1) && (btnTmr.done() )) {	// button is not pressed for a longer time, check if the double flags timed out
		//if (armFlg) dbg << "r\n";
		armFlg = lstSht = lstLng = lngRpt = dblLng = 0;

	}
}

void CB::outSignal(uint8_t mode) {
	RG::s_modTable *pModTbl = &modTbl[1];													// pointer to the respective line in the module table

	hm.pw.stayAwake(500);																	// stay awake to fulfill the action
	hm.ld.blinkRed();																		// show via led that we have some action in place
	
	#ifdef CB_DBG																			// only if ee debug is set
		if (mode == 1) dbg << F("keyShortSingle\n");										// ...and some information
		if (mode == 2) dbg << F("keyShortDouble\n");
		if (mode == 3) dbg << F("keyLongSingle\n");
		if (mode == 4) dbg << F("keyLongRepeat\n");
		if (mode == 5) dbg << F("keyLongRelease\n");
		if (mode == 6) dbg << F("keyLongDouble\n");
		if (mode == 7) dbg << F("keyLongTimeout\n");
	#endif

	if        (mode == 1) {					// keyShortSingle
		if (scn == 1) hm.sendDEVICE_INFO();													// send pairing string
		else if (scn == 2) if (pModTbl->isActive) pModTbl->mDlgt(TOOGLE);					// send toggle to user module registered on channel 1
		
	} else if (mode == 2) {					// keyShortDouble
		
	} else if (mode == 3) {					// keyLongSingle
		if      (scn == 1) hm.ld.set(key_long);
		else if (scn == 2) hm.sendDEVICE_INFO();											// send pairing string

	} else if (mode == 4) {					// keyLongRepeat
		hm.ld.set(nothing);

	} else if (mode == 5) {					// keyLongRelease

	} else if (mode == 6) {					// keyLongDouble
		hm.ld.set(nothing);

		// TODO: 0x18 localResDis available, take care of it
		uint8_t localResDis = hm.ee.getRegAddr(0,0,0,0x18);									// get register address
		//dbg << "x:" << localResDis <<'\n';
		if (!localResDis) 																	// if local reset is not disabled, reset
			hm.deviceReset(AS_RESET_CLEAR_EEPROM);
		
	}
}
