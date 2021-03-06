/*
    View: Base
*/

#ifndef _view_base_H
#define _view_base_H

#include "../../def.h"
#include <U8g2lib.h>


/* ------------------------------------------------------------------------------------------- *
 *  Дисплей
 * ------------------------------------------------------------------------------------------- */

// Пин подсветки дисплея
#define LIGHT_PIN 32

// принудительная перерисовка экрана
void displayUpdate();

/* Управление дисплеем */
void displayOn();
void displayOff();
void displayLightTgl();
bool displayLight();
void displayContrast(uint8_t value);


/* ------------------------------------------------------------------------------------------- *
 *  Кнопки
 * ------------------------------------------------------------------------------------------- */

// Пины кнопок
//#define BUTTON_PIN_UP     14
//#define BUTTON_PIN_SEL    27
//#define BUTTON_PIN_DOWN   26
#if HWVER <= 1

#define BUTTON_PIN_UP       12
#define BUTTON_PIN_SEL      14
#define BUTTON_PIN_DOWN     27

#ifdef USE4BUTTON
#define BUTTON_PIN_4        13
#endif

#define BUTTON_GPIO_PWR   GPIO_NUM_14

#else

#define BUTTON_PIN_UP       39
#define BUTTON_PIN_SEL      34
#define BUTTON_PIN_DOWN     35

#define BUTTON_GPIO_PWR   GPIO_NUM_34

#ifdef USE4BUTTON
#define BUTTON_PIN_4        12
#endif

#endif

// Коды кнопок
typedef enum {
    BTN_UP = 1,
    BTN_DOWN,
    BTN_SEL
} btn_code_t;

typedef struct {
    uint8_t     pin;
    btn_code_t  code;       // Код кнопки
    uint8_t     val;        // текущее состояние
    uint8_t     pushed;     // сработало ли уже событие pushed, чтобы повторно его не выполнить, пока кнопка не отпущена
    bool        evsmpl;
    bool        evlong;
    uint32_t    lastchg;    // millis() крайнего изменения состояния
} btn_t;

// минимальное время между изменениями состояния кнопки
#define BTN_FILTER_TIME     50
// время длинного нажатия
#define BTN_LONG_TIME       2000

// Флаги сработавших событий нажатия
#define BTN_PUSHED_SIMPLE   0x01
#define BTN_PUSHED_LONG     0x02

// время ненажатия ни на одну кнопку
uint32_t btnIdle();
// время зажатой одной или более кнопок
uint32_t btnPressed(uint8_t &btn);
// нажата ли сейчас кнопка
bool btnPushed(btn_code_t btn);

#ifdef USE4BUTTON
// нажатие на дополнительную 4ую кнопку
bool btn4Pushed();
#endif

/* ------------------------------------------------------------------------------------------- *
 *  View
 * ------------------------------------------------------------------------------------------- */

class ViewBase {
    public:
        virtual void btnSmpl(btn_code_t btn) { }
        virtual void btnLong(btn_code_t btn) { }
        virtual bool useLong(btn_code_t btn) { return false; }
        
        virtual void draw(U8G2 &u8g2) = 0;
        
        virtual void process() {}
    private:
};

void viewSet(ViewBase &v);

void viewInit();
void viewProcess();

#endif // _view_base_H
