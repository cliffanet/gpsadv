/*
    Alt calculate
*/

#ifndef __altcalc_H
#define __altcalc_H

#include <Arduino.h> 

#define AC_RR_ALT_SIZE  30
//#define PRESS_TEST 25

// states
typedef enum {
    ACST_INIT = 0,
    ACST_GROUND,
    ACST_TAKEOFF40,
    ACST_TAKEOFF,
    ACST_FREEFALL,
    ACST_CANOPY,
    ACST_LANDING
} ac_state_t;

// direct
typedef enum {
    ACDIR_INIT = 0,
    ACDIR_UP,
    ACDIR_NULL,
    ACDIR_DOWN
} ac_direct_t;

typedef struct {
    uint32_t mill;
    float press;
    float alt;
} ac_data_t;

class altcalc
{
    public:
        void tick(float press);
        // ������� �������� � �����
        const float         pressgnd()  const { return _pressgnd; }
        // ������ ����� ����� �����
        const float         altprev()   const { return _altprev; }
        // ���������� ������ (��� ����������)
        const float         altlast()   const { return _altlast; }
        // ������� ������ � ������ ������������� (����������)
        const float         alt()       const { return _altappr; }
        // ������� �������� (���������� ��������) �/�
        const float         speed()     const { return _speed; }
        // ����� ������ �� ����������� ����
        const ac_state_t    stateprev() const { return _stateprev; }
        // ������� ����� ������ (������������ �������������)
        const ac_state_t    state()     const { return _state; }
        // ����������� ������������� �������� (�����/����)
        const ac_direct_t   direct()    const { return _dir; }
        // ��� ����� ����������� ������� ����������� �������� (� ����������� �����)
        const uint32_t      dircnt()    const { return _dircnt; }
        
        // ���������� "����" ������ � ������� ���������, � ��� �� �������� ��� ���������
        void gndreset();
  
    private:
        void initend();

#ifdef PRESS_TEST
        float _presstest = 0;
#endif
        float _pressgnd = 101325;
        float _altlast = 0;
        float _altprev = 0;
        float _altappr = 0;
        float _speed = 0;
        ac_state_t _state = ACST_INIT;
        ac_state_t _stateprev = ACST_INIT;
        ac_data_t _rr[AC_RR_ALT_SIZE];
        uint8_t cur = 0;
        ac_direct_t _dir = ACDIR_INIT;
        uint32_t    _dircnt = 0;
};

#endif // __altcalc_H
