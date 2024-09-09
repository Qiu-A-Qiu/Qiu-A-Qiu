#define Plus 0
#define Automation 0
#if Plus
  #include "M5StickCPlus.h"
#else
  #include "M5StickCPlus2.h"
  #include "time.h"
#endif

#include "Slot.h"

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define LOOP_WAIT 30
#define FLUSH_DELAY 100
#define FLUSH_COUNT 5

#define Time_MAX 80
#define Time_MIN 45
#define InitTime_MAX 100
#define InitTime_MIN 80
#define StartCount_MAX 6

Slot slots[SLOT_COUNT];
const int symbolIndices[] = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
enum SlotsState { SLOTS_INIT, SLOTS_START, SLOTS_STOP = SLOT_COUNT + 1, SLOTS_FLUSH };
int state = SLOTS_INIT;

//
// setup()
//

void setup() {
 	M5.begin();
	M5.Lcd.setRotation(3);
	M5.Lcd.fillScreen(TFT_BLACK);
	M5.Lcd.setSwapBytes(true);
  
  #if Plus
	M5.Axp.ScreenBreath(12);
  #endif
  
	Slot::initShadow();
	Slot::setReel(symbolIndices, _countof(symbolIndices));
	for (int i = 0; i < SLOT_COUNT; i++) {
		slots[i].init(i, i * SLOT_COUNT);//state = SLOT_INIT;
		slots[i].draw();
	}
 srand(time(0));
}

//
// loop()
//

void loop() {
	unsigned long tick = millis();
	static unsigned long flushTick;
	static int flushCount;
  static int Slot_Start = 0;
  static int Slot_Stop = 0;
  static int Count_Stop = 0;
  static int Count_Num = rand() % (InitTime_MAX - InitTime_MIN - 1) + InitTime_MIN;
  if(Automation){    
  	M5.update();
  	if (M5.BtnA.wasPressed()||Slot_Stop == 1) {
  		if (state == SLOTS_INIT) {
  			for (int i = 0; i < SLOT_COUNT; i++) {
  				slots[i].start();
  			}
  			state++;
        Slot_Start = 1;
        Count_Num = rand() % (InitTime_MAX - InitTime_MIN - 1) + InitTime_MIN;
  		} else if (state < SLOTS_STOP) {          
        Slot_Start++;
  			slots[state - 1].stop();    
        state++;
  		}
  	}
    Slot_Stop = 0;
  }else{
    M5.update();
    if (M5.BtnA.wasPressed()) {
      if (state == SLOTS_INIT) {
        for (int i = 0; i < SLOT_COUNT; i++) {
          slots[i].start();
        }
        state++;
      } else if (state < SLOTS_STOP) {
        slots[state - 1].stop();
        state++;
      }
  }  
  }
	if (state == SLOTS_STOP) {
		int symbol = -1;
		bool stopAll = true;
		for (int i = 0; i < SLOT_COUNT; i++) {
			int n = slots[i].getSymbol();
			if (n == -1) {
				stopAll = false;
			} else {
				symbol = (n == symbol || symbol == -1) ? n : -2;
			}
		}
		if (stopAll) {
			if (symbol >= 0) {
				flushTick = tick;
				flushCount = 0;
				state = SLOTS_FLUSH;
			} else {
				state = SLOTS_INIT;
			}
		}
	}
	if (state == SLOTS_FLUSH) {
		if (tick >= flushTick + FLUSH_DELAY) {
			flushTick = tick;
			for (int i = 0; i < SLOT_COUNT; i++) {
				slots[i].flush((flushCount & 1) ? TFT_WHITE : TFT_BLUE);
			}
			if (++flushCount >= FLUSH_COUNT * 2) {
				state = SLOTS_INIT;
			}
		}
	}
 if(Automation){
   if(Slot_Start && Slot_Start < StartCount_MAX){
    Count_Stop++;
    if((Count_Stop == Count_Num)&&state < SLOTS_STOP){
      Slot_Stop = 1;
      Count_Stop = 0;
      Count_Num = rand() % (Time_MAX - Time_MIN - 1) + Time_MIN;
    }
   }else if(Slot_Start == StartCount_MAX){
     Slot_Start = 0;
   }
 }
	for (int i = 0; i < SLOT_COUNT; i++) {
		if (slots[i].update()) {
			slots[i].draw();
		}
	}

	int ms = millis() - tick;
	if (ms < LOOP_WAIT) {
		delay(LOOP_WAIT - ms);
	}
 
}
