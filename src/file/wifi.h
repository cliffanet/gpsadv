/*
    WiFi-pass file functions
*/

#ifndef _file_wifi_H
#define _file_wifi_H

#include "../../def.h"

#define WIFIPASS_FILE   "wifi.txt"

/* ------------------------------------------------------------------------------------------- *
 *  Пересоздание файла с паролями
 * ------------------------------------------------------------------------------------------- */
bool wifiPassClear();

/* ------------------------------------------------------------------------------------------- *
 *  Добавление сети с паролем
 * ------------------------------------------------------------------------------------------- */
bool wifiPassAdd(const char *ssid, const char *pass);

/* ------------------------------------------------------------------------------------------- *
 *  Контрольная сумма списка сетей
 * ------------------------------------------------------------------------------------------- */
uint32_t wifiPassChkSum();

/* ------------------------------------------------------------------------------------------- *
 *  Поиск пароля по имени wifi-сети
 * ------------------------------------------------------------------------------------------- */
bool wifiPassFind(const char *ssid, char *pass = NULL);


#endif // _file_wifi_H
