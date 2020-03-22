
#include "static.h"
#include "logbook.h"
#include "wifi.h"
#include "../display.h"
#include "../gps.h"
#include "../cfg/main.h"
#include "../cfg/point.h"
#include "../cfg/jump.h"
#include "../altimeter.h"
#include "../../def.h" // time/pwr


/* ------------------------------------------------------------------------------------------- *
 *  Функции стандартного текстового вывода значения
 * ------------------------------------------------------------------------------------------- */
static void valInt(char *txt, int val) {    // числовые
    sprintf_P(txt, PSTR("%d"), val);
}
static void valOn(char *txt, bool val) {    // On/Off
    strcpy_P(txt, val ? PSTR("On") : PSTR("Off"));
}
static void valYes(char *txt, bool val) {   // Yes/No
    strcpy_P(txt, val ? PSTR("Yes") : PSTR("No"));
}
static void valDsplAuto(char *txt, int8_t val) {
    switch (val) {
        case MODE_MAIN_NONE:    strcpy_P(txt, PSTR("No change")); break;
        case MODE_MAIN_LAST:    strcpy_P(txt, PSTR("Last")); break;
        case MODE_MAIN_GPS:     strcpy_P(txt, PSTR("GPS")); break;
        case MODE_MAIN_ALT:     strcpy_P(txt, PSTR("Alti")); break;
        case MODE_MAIN_ALTGPS:  strcpy_P(txt, PSTR("Alt+GPS")); break;
        case MODE_MAIN_TIME:    strcpy_P(txt, PSTR("Clock")); break;
    }
}

/* ------------------------------------------------------------------------------------------- *
 *  Меню управления GPS-точками
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menugpsupoint[] {
    {   // Выбор текущей точки
        .name = PSTR("Current"),
        .enter = NULL,
        .showval = [] (char *txt) {
            if (pnt.numValid()) {
                valInt(txt, pnt.num());
                if (!pnt.cur().used)
                    strcpy_P(txt+strlen(txt), PSTR(" (no used)"));
            }
            else
                strcpy_P(txt, PSTR("[no point]"));
        },
        .edit = [] (int val) {
            if (val == 1)
                pnt.numDec();
            else
            if (val == -1)
                pnt.numInc();
        },
    },
    {   // Сохранение текущих координат в выбранной точке
        .name = PSTR("Save position"),
        .enter = menuFlashHold, // Сохранение только по длинному нажатию, чтобы случайно не затереть точку
        .showval = NULL,
        .edit = NULL,
        .hold = [] () {
            if (!pnt.numValid()) { // Точка может быть не выбрана
                menuFlashP(PSTR("Point not selected"));
                return;
            }
            
            TinyGPSPlus &gps = gpsGet();
            if ((gps.satellites.value() == 0) || !gps.location.isValid()) {
                // Или к моменту срабатывания длинного нажатия может не быть валидных координат (потеряны спутники)
                menuFlashP(PSTR("GPS not valid"));
                return;
            }
            
            // Сохраняем
            pnt.locSet(gps.location.lat(), gps.location.lng());
            if (!pnt.save()) {
                menuFlashP(PSTR("EEPROM fail"));
                return;
            }
            
            menuFlashP(PSTR("Point saved"));
        },
    },
    {   // Стирание координат у выбранной точки
        .name = PSTR("Clear"),
        .enter = menuFlashHold,  // Стирание только по длинному нажатию, чтобы случайно не затереть точку
        .showval = NULL,
        .edit = NULL,
        .hold = [] () {
            if (!pnt.numValid()) { // Точка может быть не выбрана
                menuFlashP(PSTR("Point not selected"));
                return;
            }
            
            // Очищаем
            pnt.locClear();
            if (!pnt.save()) {
                menuFlashP(PSTR("EEPROM fail"));
                return;
            }
            
            menuFlashP(PSTR("Point cleared"));
        },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню управления экраном
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menudisplay[] {
    {   // Включение / выключение подсветки
        .name = PSTR("Light"),
        .enter = displayLightTgl,           // Переключаем в один клик без режима редактирования
        .showval = [] (char *txt) { valOn(txt, displayLight()); },
    },
    {   // Уровень контраста
        .name = PSTR("Contrast"),
        .enter = NULL,
        .showval = [] (char *txt) { valInt(txt, cfg.d().contrast); },
        .edit = [] (int val) {
            int c = cfg.d().contrast;
            c+=val;
            if (c > 30) c = 30; // Значения в конфиге от 0 до 30
            if (c < 0) c = 0;
            if (cfg.d().contrast == c) return;
            displayContrast(cfg.set().contrast = c);  // Сразу же применяем настройку, чтобы увидеть наглядно результат
        },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню сброса нуля высотомера (на земле)
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menugnd[] {
    {   // принудительная калибровка (On Ground)
        .name = PSTR("Manual set"),
        .enter = menuFlashHold,  // Сброс только по длинному нажатию
        .showval = NULL,
        .edit = NULL,
        .hold = [] () {
            if (!cfg.d().gndmanual) {
                menuFlashP(PSTR("Manual not allowed"));
                return;
            }
            
            altCalc().gndreset();
            jmpReset();
            menuFlashP(PSTR("GND corrected"));
        },
    },
    {   // разрешение принудительной калибровки: нет, "на земле", всегда
        .name = PSTR("Allow mnl set"),
        .enter = [] () {
            cfg.set().gndmanual = !cfg.d().gndmanual;
        },
        .showval = [] (char *txt) { valYes(txt, cfg.d().gndmanual); },
    },
    {   // выбор автоматической калибровки: нет, автоматически на земле
        .name = PSTR("Auto correct"),
        .enter = [] () {
            cfg.set().gndauto = !cfg.d().gndauto;
        },
        .showval = [] (char *txt) { strcpy_P(txt, cfg.d().gndauto ? PSTR("On GND") : PSTR("No")); },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню автоматизации экранами с информацией
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menuinfo[] {
    {   // переключать ли экран в падении автоматически в отображение только высоты
        .name = PSTR("Auto FF-screen"),
        .enter = [] () {
            cfg.set().dsplautoff = !cfg.d().dsplautoff;
        },
        .showval = [] (char *txt) { valYes(txt, cfg.d().dsplautoff); },
    },
    {   // куда переключать экран под куполом: не переключать, жпс, часы, жпс+высота (по умолч)
        .name = PSTR("On CNP"),
        .enter = NULL,
        .showval = [] (char *txt) { valDsplAuto(txt, cfg.d().dsplcnp); },
        .edit = [] (int val) {
            if (val == -1) {
                if (cfg.d().dsplcnp > MODE_MAIN_NONE)
                    cfg.set().dsplcnp--;
                else
                    cfg.set().dsplcnp = MODE_MAIN_MAX;
            }
            else
            if (val == 1) {
                if (cfg.d().dsplcnp < MODE_MAIN_MAX)
                    cfg.set().dsplcnp ++;
                else
                    cfg.set().dsplcnp = MODE_MAIN_NONE;
            }
        },
    },
    {   // куда переключать экран после приземления: не переключать, жпс (по умолч), часы, жпс+высота
        .name = PSTR("After Land"),
        .enter = NULL,
        .showval = [] (char *txt) { valDsplAuto(txt, cfg.d().dsplland); },
        .edit = [] (int val) {
            if (val == -1) {
                if (cfg.d().dsplland > MODE_MAIN_NONE)
                    cfg.set().dsplland--;
                else
                    cfg.set().dsplland = MODE_MAIN_MAX;
            }
            else
            if (val == 1) {
                if (cfg.d().dsplland < MODE_MAIN_MAX)
                    cfg.set().dsplland ++;
                else
                    cfg.set().dsplland = MODE_MAIN_NONE;
            }
        },
    },
    {   // куда переключать экран при длительном бездействии на земле
        .name = PSTR("On GND"),
        .enter = NULL,
        .showval = [] (char *txt) { valDsplAuto(txt, cfg.d().dsplgnd); },
        .edit = [] (int val) {
            if (val == -1) {
                if (cfg.d().dsplgnd > MODE_MAIN_NONE)
                    cfg.set().dsplgnd--;
                else
                    cfg.set().dsplgnd = MODE_MAIN_MAX;
            }
            else
            if (val == 1) {
                if (cfg.d().dsplgnd < MODE_MAIN_MAX)
                    cfg.set().dsplgnd ++;
                else
                    cfg.set().dsplgnd = MODE_MAIN_NONE;
            }
        },
    },
    {   // куда переключать экран при включении: запоминать после выключения, все варианты экрана
        .name = PSTR("Power On"),
        .enter = NULL,
        .showval = [] (char *txt) { valDsplAuto(txt, cfg.d().dsplpwron); },
        .edit = [] (int val) {
            if (val == -1) {
                if (cfg.d().dsplpwron > MODE_MAIN_LAST)
                    cfg.set().dsplpwron--;
                else
                    cfg.set().dsplpwron = MODE_MAIN_MAX;
            }
            else
            if (val == 1) {
                if (cfg.d().dsplpwron < MODE_MAIN_MAX)
                    cfg.set().dsplpwron ++;
                else
                    cfg.set().dsplpwron = MODE_MAIN_LAST;
            }
        },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню управления часами
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menutime[] {
    {   // Выбор временной зоны, чтобы корректно синхронизироваться с службами времени
        .name = PSTR("Zone"),
        .enter = NULL,
        .showval = [] (char *txt) {
            if (cfg.d().timezone == 0) {            // cfg.timezone хранит количество минут в + или - от UTC
                strcpy_P(txt, PSTR("UTC"));     // при 0 - это время UTC
                return;
            }
            *txt = cfg.d().timezone > 0 ? '+' : '-';// Отображение знака смещения
            txt++;
        
            uint16_t m = abs(cfg.d().timezone);     // в часах и минутах отображаем пояс
            txt += sprintf_P(txt, PSTR("%d"), m / 60);
            m = m % 60;
            sprintf_P(txt, PSTR(":%02d"), m);
        },
        .edit = [] (int val) {
            if (val == -1) {
                if (cfg.d().timezone >= 12*60)      // Ограничение выбора часового пояса
                    return;
                cfg.set().timezone += 30;             // часовые пояса смещаем по 30 минут
                adjustTime(30 * 60);            // сразу применяем настройки
                                            // adjustTime меняет время от текущего,
                                            // поэтому надо передавать не абсолютное значение,
                                            // а смещение от текущего значения, т.е. по 30 минут
            }
            else
            if (val == 1) {
                if (cfg.d().timezone <= -12*60)
                    return;
                cfg.set().timezone -= 30;
                adjustTime(-30 * 60);
            }
        },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню управления питанием
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menupower[] {
    {
        .name = PSTR("Off"),
        .enter = menuFlashHold,     // Отключение питания только по длинному нажатию, чтобы не выключить случайно
        .showval = NULL,
        .edit = NULL,
        .hold = pwrOff
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Меню управления остальными системными настройками
 * ------------------------------------------------------------------------------------------- */
static const menu_el_t menusystem[] {
    {
        .name = PSTR("Factory reset"),
        .enter = menuFlashHold,     // Сброс настроек только по длинному нажатию
        .showval = NULL,
        .edit = NULL,
        .hold = [] () {
            if (!cfgFactory()) {
                menuFlashP(PSTR("EEPROM fail"));
                return;
            }
            menuFlashP(PSTR("Config reseted"));
            ESP.restart();
        },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Главное меню конфига, тут в основном только подразделы
 * ------------------------------------------------------------------------------------------- */
#define SUBMENU(menu) [] () { menuEnter(new MenuStatic(menu, sizeof(menu)/sizeof(menu_el_t))); }
static const menu_el_t menumain[] {
    {
        .name = PSTR("GPS points"),
        .enter = SUBMENU(menugpsupoint),
    },
    {
        .name = PSTR("Jump count"),
        .enter = NULL,
        .showval = [] (char *txt) { valInt(txt, jmp.d().count); },
        .edit = [] (int val) {
            int32_t c = jmp.d().count + val;
            if (c < 0) c = 0;
            if (c == jmp.d().count) return;
            jmp.set().count = c;
        },
    },
    {
        .name = PSTR("LogBook"),
        .enter = [] () { menuEnter(new MenuLogBook); },
    },
    {
        .name = PSTR("Display"),
        .enter = SUBMENU(menudisplay),
    },
    {
        .name = PSTR("Gnd Correct"),
        .enter = SUBMENU(menugnd),
    },
    {
        .name = PSTR("Auto Screen-Mode"),
        .enter = SUBMENU(menuinfo),
    },
    {
        .name = PSTR("Time"),
        .enter = SUBMENU(menutime),
    },
    {
        .name = PSTR("Power"),
        .enter = SUBMENU(menupower),
    },
    {
        .name = PSTR("System"),
        .enter = SUBMENU(menusystem),
    },
    {
        .name = PSTR("Wifi sync"),
        .enter = [] () { menuEnter(new MenuWiFi); },
    },
};

/* ------------------------------------------------------------------------------------------- *
 *  Описание методов шаблона класса MenuStatic
 * ------------------------------------------------------------------------------------------- */
MenuStatic::MenuStatic() :
    MenuStatic(menumain, sizeof(menumain)/sizeof(menu_el_t), PSTR("Configuration"))
{
    
}

void MenuStatic::updStr(menu_dspl_el_t &str, int16_t i) {
    Serial.printf("MenuStatic::updStr: %d\r\n", i);
    auto &m = menu[i];
    
    strncpy_P(str.name, m.name, sizeof(str.name));
    str.name[sizeof(str.name)-1] = '\0';
    
    if (m.showval == NULL)
        str.val[0] = '\0';
    else
        m.showval(str.val);
}

void MenuStatic::btnSmp() {
    auto &m = menu[sel()];
    if (m.enter != NULL)
        m.enter();
}

bool MenuStatic::useLng() {
    return menu[sel()].hold != NULL;
}

void MenuStatic::btnLng() {
    auto &m = menu[sel()];
    if (m.hold != NULL)
        m.hold();
}
bool MenuStatic::useEdit() {
    return menu[sel()].edit != NULL;
}
void MenuStatic::edit(int val) {
    auto &m = menu[sel()];
    if (m.edit != NULL)
        m.edit(val);
}