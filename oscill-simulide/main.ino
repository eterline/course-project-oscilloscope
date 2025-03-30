#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include <GyverButton.h>

// ===========================================================================

#define SCREEN_WIDTH  128 // Ширина экрана в точках
#define SCREEN_HEIGHT 64  // Высота экрана в точках

#define ADC_READ_PIN  PB3 // Пин пробы замеров

#define STORE_BTN_PIN PB1 // Пин для управление памтью
#define SCALE_BTN_PIN PB4 // Пин масштабирования изображения
#define MODE_BTN_PIN  PB5 // Пин режима

#define STORE_SIZE    1

// ===========================================================================

class MilTimer {
	private:
		unsigned long tNext = 0, intervalTime;

	public:
		MilTimer(unsigned long time) : intervalTime(time) {}
		bool isDone() {
			unsigned long now = millis();

			boolean state = now - tNext >= intervalTime;
			if (state) tNext = now;

			return state;
		}
};

// ===========================================================================

// Буфер для экрана
uint8_t GraphicArray[SCREEN_WIDTH];
volatile uint8_t scaleFactor = 1;

// Хранилище значений в массиве
uint8_t SaveArray[STORE_SIZE][SCREEN_WIDTH];
int SaveArrShift = 0;

boolean saveState = false;
boolean viewMode = false;

MilTimer time(1500); // Таймер для измерений

// Установки парамтеров кнопок управления
GButton modeBtn(MODE_BTN_PIN);
GButton storeBtn(STORE_BTN_PIN);
GButton scaleBtn(SCALE_BTN_PIN);

// ===========================================================================

void setup() {
    oled.begin(
		128, 64,
		sizeof(tiny4koled_init_128x64br),
		tiny4koled_init_128x64br
	);
    oled.clear();
    oled.on();

	modeBtn.setTimeout(500);
	storeBtn.setTimeout(500);
  	scaleBtn.setTimeout(500);
}

void loop() {
	modeBtn.tick();
	storeBtn.tick();
	scaleBtn.tick();
	
	if (scaleBtn.isSingle()) scaleFactor = scaleFactor + 1; // Масштабируем ихображение
  	if (scaleBtn.isHold()) scaleFactor = 1; // Сброс масштаба на экране

  	// Смена режима отображения из памяти или чтения канала
	if (modeBtn.isSingle()) { viewMode = !viewMode; return; }
  
	// Переключаем секцию памяти через кольцевой буфер
	if (modeBtn.isSingle()) { SaveArrShift = (SaveArrShift + 1) % STORE_SIZE; return; }

 	// Сохраняем уже отображаемый фрейм с графиком в буфер, если стоит режим чтения канала
  	if (storeBtn.isHold() && !viewMode) { saveState = true; return; }
	
  // Проверка на режим чтения памяти, если да, то делаем отображение по запросу
	if (viewMode) 
	{
		if (storeBtn.isSingle()) {
			readSavedADC(GraphicArray);
			oledDisplay(GraphicArray, scaleFactor, false);
		}
		return;
	}

  // Стандартное отображение в графика
	if (time.isDone())
	{	
		if (!saveState) readADC(GraphicArray);
		oledDisplay(GraphicArray, scaleFactor, saveState);
    	saveState = false;
	}
}

// ===========================================================================

// readSavedADC - читает из памяти со сдвигом в буфер для экрана
void readSavedADC(uint8_t *arr) {
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
	{
		arr[x] = SaveArray[SaveArrShift][x];
	}
}

// readADC - формирует массив значений ADC
void readADC(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
	    uint16_t value = analogRead(ADC_READ_PIN);
      	arr[x] = value>>4;
    }
}

// oledDisplay - отобрпажаем массив в память
void oledDisplay(uint8_t* valueArr, uint8_t scale, boolean isSave) {
	oled.clear();
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
	
		uint8_t pos = x / scale;
		
    	// Сохрканение отображаемого массива в память по инедксам
		if (isSave) SaveArray[SaveArrShift][x] = GraphicArray[pos];
	
	  	// Вычисление страницы и бита
		// Ограничиваем значение от 0 до 63
		uint8_t value = (valueArr[pos] > 63) ? 63 : valueArr[pos];
		
	    uint8_t page = value / 8;    			// Определяем страницу (каждая страница 8 пикселей)
	    uint8_t shiftBit = 1 << (value % 8);   	// Определяем бит в странице
	
		// Определеяем страницу пространства экрана
	    oled.setCursor(x, page);
	
		// Устанавливаем курсор в нужную позицию
	    oled.startData();
	    oled.sendData(shiftBit);
	    oled.endData();
	}
}