/*
    Menu Base class
*/

#ifndef _menu_static_H
#define _menu_static_H

#include "base.h"
#include "../mode.h"

typedef void (*menu_hnd_t)();
typedef void (*menu_val_t)(char *txt);

// ������� ����
typedef struct {
    char        *name;      // ��������� �������� ������
    menu_hnd_t  enter;      // �������� ������� �� ������� ������
    menu_val_t  showval;    // ��� ���������� ��������, ���� ���������
    menu_hnd_t  editup;     // � ������ �������������� ������ �������� ����� (������� ������ ���������, ��� � ��� ������)
    menu_hnd_t  editdown;   // � ������ �������������� ������ �������� ����
    menu_hnd_t  hold;       // ��������� �������� ��� ������� ������� �� ������� ������
} menu_el_t;


class MenuStatic : public MenuBase {
    public:
        MenuStatic();
        MenuStatic(const menu_el_t *m, int16_t sz, const char *_title = NULL) : MenuBase(sz, _title), menu(m) {};
        void updStr(menu_dspl_el_t &str, int16_t i);
        
        void updHnd(int16_t i, button_hnd_t &smp, button_hnd_t &lng, button_hnd_t &editup, button_hnd_t &editdn);
    
    private:
        const menu_el_t *menu;
};

#endif // _menu_static_H
