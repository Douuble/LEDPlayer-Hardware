//暂时废弃：内存问题尚未找到好的解决办法
//SD卡的读写硬性要求512Bytes的RAM
//这部分内存不便共享
/*#include <SD.h>

#define CS_PIN 10
File f;

void SDFunc() {
}

inline bool SDInit() {  
  if (!SD.begin(CS_PIN)) {
    drawMenu(sdDetectFail);
    while (KeyManager() != 3);
    return false;
  }
  
}

inline void SDProcess() {  
  if (!SD.begin(CS_PIN)) {
    drawMenu(sdDetectFail);
    while (KeyManager() != 3);
  }
}*/
