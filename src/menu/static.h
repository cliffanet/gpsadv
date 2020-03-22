/*
    Menu Base class
*/

#ifndef _menu_static_H
#define _menu_static_H

#include "base.h"
#include "../mode.h"

typedef void (*menu_hnd_t)();
typedef void (*menu_edit_t)(int val);
typedef void (*menu_val_t)(char *txt);

// ������� ����
typedef struct {
    char        *name;      // ��������� �������� ������
    menu_hnd_t  enter;      // �������� ������� �� ������� ������
    menu_val_t  showval;    // ��� ���������� ��������, ���� ���������
    menu_edit_t edit;       // � ������ �������������� ������ �������� �� �������� val (������� ������ ���������, ��� � ��� ������)
    menu_hnd_t  hold;       // ��������� �������� ��� ������� ������� �� ������� ������
} menu_el_t;


class MenuStatic : public MenuBase {
    public:
        MenuStatic();
        MenuStatic(const menu_el_t *m, int16_t sz, const char *_title = NULL) : MenuBase(sz, _title), menu(m) {};
        void updStr(menu_dspl_el_t &str, int16_t i);
        
        void btnSmp();
        bool useLng();
        void btnLng();
        bool useEdit();
        void editEnter() { btnSmp(); }
        void edit(int val);
    
    private:
        const menu_el_t *menu;
};

#endif // _menu_static_H
