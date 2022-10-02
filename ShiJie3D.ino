/*视界光立方 Arduino控制代码 v1.0*/
/*19041106 杨瑶*/
/*19080221 杨彬*/
/*19080204 蒋胜圣*/
/*2021年12月03日*/

/*敬请注意：微处理器型号限定为ATMega328P。
  本程序为了性能最大化，使用大量汇编级语句，
  直接操作寄存器。不具有可移植性！
  请勿在非328P微处理器的开发板上直接运用！*/

/*文件说明*/
#include "BusLogic.h"
#include "Font.h"
#include "LcdUI.h"

/*当前运行状态*/
uint8_t State = 0xFF;
#define MAIN_MENU 0
#define USB_SERIAL 1
#define SD_CARD 2
#define AUDIO_INPUT 3
#define WIFI_SERIAL 4

/*状态转换。*/
inline void stateChange(register uint8_t p) {
  if (p == MAIN_MENU) {//如果目的是回主菜单
    switch (State) {
      case WIFI_SERIAL:
      case USB_SERIAL:
        Serial.end();//若当前状态用到串口，则停用串口
      case SD_CARD:
      case AUDIO_INPUT:
        zeroBuffer();
        break;
    }
    drawMenu(startUp);
    while (myAnalogRead(2) <= pgm_read_byte_near(KeyVal + 5));
  }
  else if (p == USB_SERIAL) {
    serialReset();
    Serial.begin(250000);
    drawMenu(usbOK);
  }
  else if (p == WIFI_SERIAL) {
    serialReset();
    Serial.begin(115200);
    drawMenu(wifiShow);
  }
  else if (p == AUDIO_INPUT) {
    drawMenu(audioOK);
  }
  State = p;
}

void setup() {
  portInit();
  myAnalogInit();
  LcdUIInit();
  drawMenu(welcome1);//显示欢迎画面1
  delay(1500);
  drawMenu(welcome2);//显示欢迎画面2
  delay(3500);
  stateChange(MAIN_MENU);
}

void loop() {
  switch (State) {
    case MAIN_MENU:     //****archived****

      { register uint8_t cur = 1;//光标位置
        register uint8_t usb_wifi = (myAnalogRead(2) <= pgm_read_byte_near(KeyVal + 6));
        drawMenu(usb_wifi ? mainMenuWiFi : mainMenuUSB);
        lcd.setCursor(0, cur); lcd.write('~');
        for (; State == MAIN_MENU;) {
          switch (KeyManager()) {
            case 1: lcd.setCursor(0, cur); lcd.write(' '); --cur; if (cur < 1)cur = 3; lcd.setCursor(0, cur); lcd.write('~'); break;
            case 4: lcd.setCursor(0, cur); lcd.write(' '); ++cur; if (cur > 3)cur = 1; lcd.setCursor(0, cur); lcd.write('~'); break;
            case 5: if (cur == 1 && usb_wifi)cur = 4; stateChange(cur); break;
            case 6: if (!usb_wifi) {
                drawMenu(mainMenuWiFi);
                usb_wifi = 1;
                lcd.setCursor(0, cur);
                lcd.write('~');
              } break;
            case 7: if (usb_wifi) {
                drawMenu(mainMenuUSB);
                usb_wifi = 0;
                lcd.setCursor(0, cur);
                lcd.write('~');
              } break;
          }
        }
      }
      break;


    case USB_SERIAL:    //****archived****
      for (;;) {
        fullScan(serialFunc);
        if (KeyManager() == 3) {
          stateChange(MAIN_MENU);
          break;
        }
      }
      break;


    case SD_CARD:

      break;


    case AUDIO_INPUT:     //****archived****
      for (;;) {
        scan(1125, sampleFunc);
        sampleProc();
        if (KeyManager() == 3) {
          stateChange(MAIN_MENU);
          break;
        }
      }
      break;


    case WIFI_SERIAL:
      for (;;) {
        fullScan(serialFunc);
        if (KeyManager() == 3) {
          stateChange(MAIN_MENU);
          break;
        }
      }
      break;
  }
}
