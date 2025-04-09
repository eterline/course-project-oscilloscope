#include <TinyWireM.h>
#include <Tiny4kOLED.h>
#include <GyverButton.h>
#include <EEPROM.h>

// ===========================================================================

#define SCREEN_WIDTH  	128 // Ширина экрана в точках
#define SCREEN_HEIGHT 	64  // Высота экрана в точках

#define ADC_READ_PIN  	PB3 // Пин пробы замеров

#define STORE_BTN_PIN 	PB1 // Пин для управление памятью
#define SCALE_BTN_PIN 	PB4 // Пин масштабирования изображения
#define MODE_BTN_PIN  	PB5 // Пин режима

#define STORE_SIZE 		2   // Размер кольцевого буфера памяти

// ===========================================================================

// Класс для работы с таймером, упрощает логику работы с ним
// сделано для избавления от прерываний потока управления на delay();
class MilTimer {
	private:
		unsigned long tNext = 0, intervalTime;

	public:
		// MilTimer - констуктор класса, задается интервал срабатывания в мс
		MilTimer(unsigned long time) : intervalTime(time) {}
		
		// isDone - проверка прохождения таймером интервала и обновление счетчика
		bool isDone() {
			unsigned long now = millis();

			boolean state = now - tNext >= intervalTime;
			if (state) tNext = now;

			return state;
		}
};

// ===========================================================================
MilTimer time(1500); // Таймер для измерений - 1.5 секунд

// Установки парамтеров кнопок управления ====
GButton modeBtn(MODE_BTN_PIN);		// Кнопка смены режма работы устиройства
GButton storeBtn(STORE_BTN_PIN);		// Кнопка работы с памятью
GButton scaleBtn(SCALE_BTN_PIN);		// Кнопка масштабирования изображения

// Параметры отображения ===
uint8_t scaleFactor = 1; 				// Значение масштабирования графика
uint8_t GraphicBuffer[SCREEN_WIDTH]; 	// Буфер для экрана в массиве

boolean viewMode = false;	// Режим отображения

// Состояние запроса на сохранение графика.
// По умолчанию - false, при переключении сохраняется буфер в память и значение сбрасыавется в стоковое
boolean saveState = false;

// Индекс сдвига по памяти для работы с ее сегментами
uint8_t memorySaveShift = 0;

// Переключение секции памяти через кольцевой буфер
void incrementSaveShift() {memorySaveShift = (memorySaveShift + 1) % STORE_SIZE;}

// ===========================================================================

void setup() {
	
	// Инициализируем экран отображения
    oled.begin(
		128, 64,
		sizeof(tiny4koled_init_128x64br),
		tiny4koled_init_128x64br
	);
    oled.clear();
    oled.on();

	// Преднастройки нтервалов для простоты работы и удобства, ничего более
	modeBtn.setTimeout(500);
	storeBtn.setTimeout(500);
  	scaleBtn.setTimeout(500);
}

void loop() {

    // Обработка кнопок в начале каждого цикла
    modeBtn.tick();
    storeBtn.tick();
    scaleBtn.tick();

	// ========================

	// Масштабируем изображение
    if (scaleBtn.isSingle()) scaleFactor = scaleFactor + 1;

	// Сброс масштаба на экране
    if (scaleBtn.isHold()) scaleFactor = 1; 

	// Сброс EEPROM
    if (scaleBtn.isTriple()) for (uint8_t i = 0; i < 128 * STORE_SIZE; i++) EEPROM.write(i, 0);

    // Смена режима отображения из памяти или чтения пробы
    if (modeBtn.isHold()) {
        viewMode = !viewMode;
        return;
    }

	if (modeBtn.isSingle()) {
        incrementSaveShift();
        return;
    }

    // Сохраняем уже отображаемый фрейм с графиком в буфер, если стоит режим чтения канала
	// Задается состояние переменной статуса сохранения, сохранение проихсодит на следующем цикле
	// Сохраняется уже отображенный фрейм графика, не новый
    if (storeBtn.isHold() && !viewMode) {
        saveState = true;
        return;
    }

	// Проверка на режим чтения памяти, если да, то делаем отображение по запросу
    if (storeBtn.isSingle() && viewMode) {
        readADCfromMEM(GraphicBuffer);
        oledDisplay(GraphicBuffer, scaleFactor);
    }

	// ========================

    // Стандартное отображение графика
    if (!viewMode && time.isDone()) {
        if (saveState) {
            saveState = false;
            saveADCtoMEM(GraphicBuffer);
        } else {
            readADC(GraphicBuffer);
            oledDisplay(GraphicBuffer, scaleFactor);
        }
    }
}

// ===========================================================================

// readSavedADC - читает из памяти со сдвигом в буфер для экрана
void readADCfromMEM(uint8_t *arr) {
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
	{
		arr[x] = EEPROM.read(memorySaveShift * 128 + x);
	}
}

// saveADCtoMEM - сохраняет из буффера в память
void saveADCtoMEM(uint8_t *arr) {
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++)
	{
		EEPROM.write(memorySaveShift * 128 + x, arr[x]);
	}
}

// readADC - формирует массив значений ADC
void readADC(uint8_t *arr) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
	    uint16_t value = analogRead(ADC_READ_PIN);
      	arr[x] = uint8_t(value >> 4);
    }
}

// oledDisplay - отобрпажаем массив в память дисплея
void oledDisplay(uint8_t* valueArr, uint8_t scale) {
	oled.clear();
	for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
		uint8_t y = valueArr[x / scale];
		setOledDot(x, 31); // Отрисовка центральной линии
	    setOledDot(x, y);
	}
}

// setOledDot - установки точки на нужной координате
void setOledDot(uint8_t x, uint8_t y) {

	// Ограничиваем значение по высоте отрисовки
	y = (y > 63) ? 63 : y;
	
	// Определяем страницу (каждая страница 8 пикселей)
	uint8_t page = y / 8;
	// Определяем бит в странице
    uint8_t shiftBit = 1 << (y % 8);

	// Устанавливаем страницу пространства экрана
    oled.setCursor(x, page);

	// Устанавливаем бит в позицию
    oled.startData();
    oled.sendData(shiftBit);
    oled.endData();
}