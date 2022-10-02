/*3D LED 显示内存*/
/*10x10x10共1000个RGB全彩灯珠*/
/*每个灯珠具有红、绿、蓝3种颜色，相当于需控制3000个单色灯珠*/
/*每种颜色具有16种亮度等级，需占用4bit
  红、绿、蓝3种分量需要12bit=1.5Byte的RAM。
  1000个灯的控制需要1500Byte的RAM
  每个灯珠可以表示的颜色共16x16x16=4096种*/
volatile uint8_t realBuff[1500];
volatile uint8_t* _3dBuf = realBuff;

/*汇编NOP指令，什么都不做，耗费1个CPU周期，精确控制时序用*/
#define NOP do { __asm__ __volatile__ ("nop"); } while (0);

/*每层刷新时的点亮时间，单位为微秒。*/
#define REFRESH_US 16

/*全屏清零*/
inline void zeroBuffer(register uint16_t s = 1500) {
  for (register uint16_t i = 0; i != s; ++i)_3dBuf[i] = 0;
}
