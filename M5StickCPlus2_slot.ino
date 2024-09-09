#include <M5StickCPlus2.h>
#include "Slot.h"

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

#define LOOP_WAIT 30
#define FLUSH_DELAY 100
#define FLUSH_COUNT 3

Slot slots[SLOT_COUNT];
const int symbolIndices[] = { 2, 4, 5, 0, 3, 4, 2, 5, 3, 1, 0};//列图标显示顺序  每一列显示的图标相同

enum SlotsState { SLOTS_INIT, SLOTS_START, SLOTS_STOP = SLOT_COUNT + 1, SLOTS_FLUSH };
int state = SLOTS_INIT;//老虎机状态值     设置为初始状态

//
// setup()//初始化函数
//
void setup() {
 	M5.begin();
	M5.Lcd.setRotation(3);
	M5.Lcd.fillScreen(TFT_BLACK);//设置屏幕背景色为黑色
	M5.Lcd.setSwapBytes(true);


	Slot::initShadow();//初始化阴影
	Slot::setReel(symbolIndices, _countof(symbolIndices));//设置初始转盘
	for (int i = 0; i < SLOT_COUNT; i++) {
		slots[i].init(i, i * SLOT_COUNT);
		slots[i].draw();
	}
}

//
// loop()
//
void loop() {
	unsigned long tick = millis();
	static unsigned long flushTick;
	static int flushCount;

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
