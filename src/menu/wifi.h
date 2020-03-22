/*
    Menu WiFi class
*/

#ifndef _menu_wifi_H
#define _menu_wifi_H

#include "base.h"
#include <vector>

typedef struct {
    char name[33];
    char txt[40];
    bool isopen;
} wifi_t;


class MenuWiFi : public MenuBase {
    public:
        MenuWiFi();
        ~MenuWiFi();
        void updStr(menu_dspl_el_t &str, int16_t i);
        
    private:
        std::vector<wifi_t> wifiall;
};

#endif // _menu_wifi_H
