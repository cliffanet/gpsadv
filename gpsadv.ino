
#include "def.h"

#include "src/log.h"
#include "src/power.h"
#include "src/clock.h"
#include "src/view/main.h"
#include "src/gps/proc.h"
#include "src/jump/proc.h"
#include "src/file/track.h"
#include "src/cfg/main.h"
#include "src/cfg/point.h"
#include "src/cfg/jump.h"

#include <SPIFFS.h>

//------------------------------------------------------------------------------
void setup() {

#ifdef FWVER_DEBUG
    Serial.begin(115200);
#endif // FWVER_DEBUG
    
    if (!pwrCheck())
        return;
    
    CONSOLE("Firmware " FWVER_FILENAME "; Build Date: " __DATE__);

    // часы
    clockInit();

    // инициируем view
    viewInit();
    
    // инициализируем высотомер
    jmpInit();

    // инициируем gps
    gpsInit();

    // загружаем сохранённый конфиг
    if(!SPIFFS.begin(true))
        CONSOLE("SPIFFS Mount Failed");
    cfgLoad(true);
    CONSOLE("begin");
}

//------------------------------------------------------------------------------

void loop() {
    uint32_t m = millis();
    clockProcess();
    gpsProcess();
    jmpProcess();
    trkProcess();
    viewProcess();
    
    uint32_t md = millis()-m;
    if (md < 100)
        delay(100-md);
    
    m = millis();
    jmpProcess();
    
    md = millis() - m;
    if (md < 100)
        delay(100-md);
}
