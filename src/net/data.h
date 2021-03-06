/*
    Data convert function
*/

#ifndef _net_data_H
#define _net_data_H

#include "../../def.h"

#include "../file/log.h"
#include "../clock.h"

typedef struct __attribute__((__packed__)) {
    uint32_t i;
    uint32_t d;
} dnet_t;

typedef struct __attribute__((__packed__)) {
    uint32_t ckscfg;
    uint32_t cksjmp;
    uint32_t ckspnt;
    uint32_t ckslog;
    uint32_t poslog;
    logchs_t ckstrack;
} daccept_t;

/* ------------------------------------------------------------------------------------------- *
 *  простые преобразования данных
 * ------------------------------------------------------------------------------------------- */
uint8_t bton(bool val);
bool ntob(uint8_t n);

uint16_t fton(float val);
float ntof(uint16_t n);

dnet_t dton(double val);

tm_t tmton(const tm_t &tm);
tm_t ntotm(const tm_t &n);

logchs_t ckston(const logchs_t &cks);
logchs_t ntocks(const logchs_t &n);

/* ------------------------------------------------------------------------------------------- *
 *  строковые преобразования
 * ------------------------------------------------------------------------------------------- */
int ntostrs(char *str, int strsz, uint8_t *buf, uint8_t **bufnext = NULL);

/* ------------------------------------------------------------------------------------------- *
 *  Отправка данных
 * ------------------------------------------------------------------------------------------- */
bool sendCfg();
bool sendJump();
bool sendPoint();
bool sendLogBook(uint32_t _cks, uint32_t _pos);
bool sendTrack(logchs_t _cks);
bool sendDataFin();


#endif // _net_data_H
