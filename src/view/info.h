/*
    View: Info
*/

#ifndef _view_info_H
#define _view_info_H

#include "base.h"

typedef enum {
    TXT_NONE,
    TXT_LEFT,
    TXT_RIGHT,
    TXT_CENTER
} viewinfo_txtdir_t;

#define PRNL(txt, ...) prnl(PSTR(txt), ##__VA_ARGS__)
#define PRNR(txt, ...) prnr(PSTR(txt), ##__VA_ARGS__)
#define PRNC(txt, ...) prnc(PSTR(txt), ##__VA_ARGS__)

class ViewInfo : public ViewBase {
    public:
        ViewInfo(uint16_t _sz, const char *_title = NULL) : sz(_sz), title(_title) {}
        void setSize(uint16_t _sz) { sz = _sz;};
        
        virtual
        void updStr(uint16_t i) {}
        void vprn(viewinfo_txtdir_t dir, const char *s, va_list ap);
        void prnl(const char *s, ...);
        void prnr(const char *s, ...);
        void prnc(const char *s, ...);
        
        void btnSmpl(btn_code_t btn);
        
        void draw(U8G2 &u8g2);
    
    private:
        uint16_t itop=0, sz;
        const char *title;
        U8G2 *_u8g2;
        uint8_t iprn=0;
};

void setViewInfoDebug();

#endif // _view_info_H
