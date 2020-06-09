
#include "track.h"
#include "../altimeter.h"
#include "../gps.h"
#include "log.h"

static trk_running_t state = TRKRUN_NONE;
File fh;

/* ------------------------------------------------------------------------------------------- *
 *  Запуск трекинга
 * ------------------------------------------------------------------------------------------- */
static bool trkCheckAvail(bool removeFirst = false);
bool trkStart(bool force) {
    if (state != TRKRUN_NONE)
        trkStop();
    
    if (!trkCheckAvail(true))
        return false;
    
    const auto *_fname = PSTR(TRK_FILE_NAME);
    if (!logRotate(_fname, 0))
        return false;
    
    char fname[36];
    
    fname[0] = '/';
    strncpy_P(fname+1, _fname, 30);
    fname[30] = '\0';
    const byte flen = strlen(fname);
    sprintf_P(fname+flen, PSTR(LOGFILE_SUFFIX), 1);
    fh = DISKFS.open(fname, FILE_WRITE);
    if (!fh)
        return false;
    
    state = force ? TRKRUN_FORCE : TRKRUN_AUTO;
    Serial.print(F("track started "));
    Serial.println(force ? F("force") : F("auto"));
    return true;
}

/* ------------------------------------------------------------------------------------------- *
 *  Остановка
 * ------------------------------------------------------------------------------------------- */
size_t trkStop() {
    if (state == TRKRUN_NONE)
        return -1;
    
    fh.close();
    
    state = TRKRUN_NONE;
    Serial.println(F("track stopped"));
    return 0;
}

/* ------------------------------------------------------------------------------------------- *
 *  Текущее состояние
 * ------------------------------------------------------------------------------------------- */
bool trkRunning() { return state > TRKRUN_NONE; }
trk_running_t trkState() { return state; }

/* ------------------------------------------------------------------------------------------- *
 *  Сколько файлов есть в наличии
 * ------------------------------------------------------------------------------------------- */
int trkFileCount() {
    return logCount(PSTR(TRK_FILE_NAME));
}

/* ------------------------------------------------------------------------------------------- *
 *  Под сколько ещё записей доступно место
 * ------------------------------------------------------------------------------------------- */
size_t trkCountAvail() {
    size_t used = DISKFS.usedBytes();
    if (used >= ALL_SIZE_MAX)
        return 0;
    return (ALL_SIZE_MAX - used) / sizeof(struct log_item_s <log_item_t>);
}

/* ------------------------------------------------------------------------------------------- *
 *  Проверка и освобождение места (удаление старых записей)
 * ------------------------------------------------------------------------------------------- */
static bool trkCheckAvail(bool removeFirst) {
    auto av = trkCountAvail();
    Serial.printf("avail: %d records\r\n", av);
    while (av < TRK_PRESERV_COUNT) {
        int avail = logRemoveLast(PSTR(TRK_FILE_NAME), removeFirst);
        if (avail <= 0) {
            Serial.println(F("track nothing to remove"));
            return false;
        }
        av = trkCountAvail();
    }
    
    return true;
}

/* ------------------------------------------------------------------------------------------- *
 *  Запись (на каждый тик тут приходится два тика высотомера и один тик жпс)
 * ------------------------------------------------------------------------------------------- */
void trkProcess() {
    if (state == TRKRUN_NONE)
        return;
    
    if (!fh)
        return;
    
    struct log_item_s <log_item_t> log;
    log.data = jmpLogItem();
    
    static uint8_t cnt = 0;
    if ((((cnt++) & 0b111) == 0) && !trkCheckAvail()) {
        trkStop();
        return;
    }
    
    auto sz = fh.write(reinterpret_cast<const uint8_t *>(&log), sizeof(log));
    if (sz != sizeof(log))
        trkStop();
}
