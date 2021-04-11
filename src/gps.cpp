
#include "gps.h"

#include <TinyGPS++.h>
static TinyGPSPlus gps;

// Доп модули, необходимые для режима DPS-Direct
#include "display.h"
#include "button.h"
#include "mode.h"
#include "menu/base.h"

// будем использовать стандартный экземпляр класса HardwareSerial, 
// т.к. он и так в системе уже есть и память под него выделена
// Стандартные пины для свободного аппаратного Serial2: 16(rx), 17(tx)
#define ss Serial2

#include "gps/ubloxproto.h"
static UbloxGpsProto gps2(ss);


/* ------------------------------------------------------------------------------------------- *
 *  GPS-обработчик
 * ------------------------------------------------------------------------------------------- */

TinyGPSPlus &gpsGet() { return gps; }

static void gpsFree() {
    gps2.uart(NULL);
}

static bool gpsUartSpeed(uint32_t baudRate) {
    // инициируем uart-порт GPS-приёмника
    ss.begin(9600);
    ss.setRxBufferSize(512); // По умолчанию = 256 и этого не хватает, чтобы принять сразу все присылаемые от GPS данные за один цикл
    
    char cnt = 5;
    
    while (cnt > 0) {
        cnt--;
        
        Serial.println("Set UART(gps) default speed 9600");
        ss.updateBaudRate(9600);
        
        struct
        {
        	uint8_t  portID;       // Port identifier number
        	uint8_t  reserved0;    // Reserved
        	uint16_t txReady;      // TX ready pin configuration
        	uint32_t mode;         // UART mode
        	uint32_t baudRate;     // Baud rate (bits/sec)
        	uint16_t inProtoMask;  // Input protocols
        	uint16_t outProtoMask; // Output protocols
        	uint16_t flags;        // Flags
        	uint16_t reserved5;    // Always set to zero
        } cfg_prt =
    	{
    		.portID       = 1,      // UART 1
    		.reserved0    = 0,      // Reserved
    		.txReady      = 0,      // no TX ready
    		.mode         = 0x08d0, // 8N1
    		.baudRate     = baudRate,  // Baudrate in bits/second
    		.inProtoMask  = 0x0001, // UBX protocol
    		.outProtoMask = 0x0001, // UBX protocol
    		.flags        = 0,      // Flags bit mask
    		.reserved5    = 0       // Reserved, set to 0
    	};
        
        if (!gps2.send(UBX_CFG, UBX_CFG_PRT, cfg_prt))
            return false;
        
        gps2.cnfclear();
        ss.flush();
        
        Serial.printf("Set UART(gps) speed %d\n", baudRate);
        ss.updateBaudRate(baudRate);
        delay(10);
        
        if (!gps2.send(UBX_CFG, UBX_CFG_PRT, cfg_prt))
            return false;
        if (gps2.waitcnf()) {
            return true;
        }
    }
    
    return false;
}

static bool gpsInitCmd() {
    if (!gpsUartSpeed(34800)) {
        Serial.println("Can't set GPS-UART baudRate");
        return false;
    }
    Serial.println("GPS-UART baudRate init ok");
    
    struct {
    	uint8_t msgClass;  // Message class
    	uint8_t msgID;     // Message identifier
    	uint8_t rate;      // Send rate
    } cfg_rate[] = {
		{ UBX_NMEA, UBX_NMEA_GPGGA,     0 },
		{ UBX_NMEA, UBX_NMEA_GPGLL,     0 },
		{ UBX_NMEA, UBX_NMEA_GPGSA,     0 },
		{ UBX_NMEA, UBX_NMEA_GPGSV,     0 },
		{ UBX_NMEA, UBX_NMEA_GPRMC,     0 },
		{ UBX_NMEA, UBX_NMEA_GPVTG,     0 },
        
		{ UBX_NAV,  UBX_NAV_POSLLH,     1 },
		{ UBX_NAV,  UBX_NAV_VELNED,     1 },
		{ UBX_NAV,  UBX_NAV_TIMEUTC,    1 },
        
        { UBX_NAV,  UBX_NAV_SOL,        1 },
        { UBX_NAV,  UBX_NAV_PVT,        1 }
    };
    
    for (auto r : cfg_rate)
        if (!gps2.send(UBX_CFG, UBX_CFG_MSG, r) || !gps2.tick())
            return false;

    struct {
    	uint16_t measRate;  // Measurement rate             (ms)
    	uint16_t navRate;   // Nagivation rate, in number 
    	                       //   of measurement cycles
    	uint16_t timeRef;   // Alignment to reference time:
    	                       //   0 = UTC time; 1 = GPS time
    } cfg_mrate = {
		.measRate   = 1000,      // Measurement rate (ms)
		.navRate    = 1,        // Navigation rate (cycles)
		.timeRef    = 0         // UTC time
	};
    if (!gps2.send(UBX_CFG, UBX_CFG_RATE, cfg_mrate) || !gps2.tick())
        return false;
	
    struct {
    	uint16_t mask;              // Only masked parameters will be applied
    	uint8_t  dynModel;          // Dynamic platform model
    	uint8_t  fixMode;           // Position fixing mode
    	int32_t  fixedAlt;          // Fixed altitude (MSL) for 2D mode       (m)
    	uint32_t fixedAltVar;       // Fixed altitude variance for 2D mode    (m^2)
    	int8_t   minElev;           // Minimum elevation for satellite        (deg)
    	uint8_t  drLimit;           // Maximum time to perform dead reckoning (s)
    	uint16_t pDop;              // Position DOP mask
    	uint16_t tDop;              // Time DOP mask
    	uint16_t pAcc;              // Position accuracy mask                 (m)
    	uint16_t tAcc;              // Time accuracy mask                     (m)
    	uint8_t  staticHoldThresh;  // Static hold threshold                  (cm/s)
    	uint8_t  res1;              // Reserved, set to 0
    	uint32_t res2;              // Reserved, set to 0
    	uint32_t res3;              // Reserved, set to 0
    	uint32_t res4;              // Reserved, set to 0
    } cfg_nav5 = {
		.mask       = 0x0001,       // Apply dynamic model settings
		.dynModel   = 7             // Airborne with < 1 g acceleration
	};
    if (!gps2.send(UBX_CFG, UBX_CFG_NAV5, cfg_nav5) || !gps2.tick())
        return false;
    
    struct {
    	uint16_t navBbrMask; // BBR sections to clear
    	uint8_t  resetMode;  // Reset type
    	uint8_t  res;        // Reserved
    } cfg_rst =	{
		.navBbrMask = 0x0000,   // Hot start
		.resetMode  = 0x09      // Controlled GPS start
	};
    if (!gps2.send(UBX_CFG, UBX_CFG_RST, cfg_rst) || !gps2.tick())
        return false;
    
    return true;
}

void gpsInit() {
    if (!gpsInitCmd()) {
        Serial.println("GPS init fail");
        gpsFree();
    }
}

void gpsProcess() {
    gps2.tick();
    //    gps.encode( ss.read() );
}

/* ------------------------------------------------------------------------------------------- *
 *  режим DPS-Direct
 * ------------------------------------------------------------------------------------------- */
static void gpsDirectExit() {
    menuClear();
    modeMain();
    
    loopMain = NULL;
}
static void gpsDirectProcess() {
    btnProcess();
    while (ss.available())
        Serial.write( ss.read() );
    while (Serial.available())
        ss.write( Serial.read() );
}
static void gpsDirectDisplay(U8G2 &u8g2) {
    char txt[128];
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
    // Заголовок
    strcpy_P(txt, PSTR("GPS -> Serial Direct"));
    u8g2.drawBox(0,0,128,12);
    u8g2.setDrawColor(0);
    u8g2.drawStr((u8g2.getDisplayWidth()-u8g2.getStrWidth(txt))/2, 10, txt);
    
    u8g2.setDrawColor(1);
    strcpy_P(txt, PSTR("To continue"));
    u8g2.drawStr((u8g2.getDisplayWidth()-u8g2.getStrWidth(txt))/2, 30+14, txt);
    strcpy_P(txt, PSTR("press SELECT-button"));
    u8g2.drawStr((u8g2.getDisplayWidth()-u8g2.getStrWidth(txt))/2, 40+14, txt);
}

void gpsDirect() {
    btnHndClear();
    btnHnd(BTN_SEL, BTN_SIMPLE, gpsDirectExit);
    
    displayHnd(gpsDirectDisplay);
    displayUpdate();
    
    loopMain = gpsDirectProcess;
}
