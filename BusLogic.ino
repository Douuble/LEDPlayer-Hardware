/*总线逻辑，灯的控制*/

/*翻转OE信号的电平。OE信号是锁存器输出使能端，
  为高电平时，锁存器全部输出置高阻态，所有灯熄灭。
  为低电平时，输出锁存的内容。*/
inline void toggleOE() {
  PORTB ^= 0x04;
}

/*翻转ALE信号的电平。ALE信号是锁存器锁存控制端，
  为高电平时，输入端的数据直通到输出端。
  切换到低电平瞬间，输入端的数据被锁存。
  直到下一个高电平来临之前，输出的都是锁存的数据。*/
inline void toggleALE() {
  PORTC ^= 0x08;
}

/*在1个时钟周期内，将A8和ALE同时翻转*/
inline void toggleALEwithA8() {
  PORTC ^= 0x09;
}

/*对各I/O进行初始化。
  用数字I/O模拟9位地址/数据复用总线，
  用一片74LS573锁存器进行复用。
  其中地址9位，数据8位。
  A0是总线的最高位，且只进行地址的传输，不参与数据传输，下文称A8地址线。
  2-7是总线的次高6位。
  8-9是总线的低2位。
  0-1保留给串口（USB和Wi-Fi）；A4、A5保留给I2C（液晶屏）；
  11-13保留给SPI（SD卡）。
  10为OE，A3为ALE，A1接运放放大过的音频信号，A2接电阻网络键盘。
  《 ATMega328P:你给我插满了是要累死我吗(^^;) 》*/
inline void portInit() {
  DDRD |= 0xFC;   //2-7置输出，0-1保持现状
  PORTD &= 0x03;  //2-7置0,0-1保持现状
  DDRB |= 0x07;   //8-10置输出，11-13保持现状
  PORTB &= 0xF8;  //8-10置0,11-13保持现状
  DDRC |= 0x09;   //A0、A3置输出，其余模拟口保持现状
  DDRC &= 0xF9;   //A1、A2置输入，其余模拟口保持现状
  PORTC &= 0xF6;  //A0、A3置0，其余模拟口保持现状
  toggleOE();     //10（OE）置1
}

/*向8位数据总线中写数据。*/
inline void busWrite(register uint8_t data) {
  register uint8_t buffD = PORTD ^ data;
  buffD &= 0xFC;
  data ^= PORTB;
  data &= 0x03;
  PORTD ^= buffD;
  PORTB ^= data;
}

/*A8地址线置1。*/
inline void A8_set() {
  PORTC |= 0x01;
}

/*A8地址线置0。*/
inline void A8_clr() {
  PORTC &= 0xFE;
}

/*对所有LED扫描一次。_base是访问_3dBuf的基址；
  callback是回调函数指针，让CPU在等延时期间有事可做。
  《 每个搞嵌入式的都有当资本家的潜质也说不定。 》*/
inline void scan(register uint16_t _base, register void (*callback)()) {
  register uint32_t t0;   //时间戳
  register uint8_t layer; //选层
  register uint8_t chip;  //选锁存器片
  register uint16_t base;
  for (layer = 0; layer != 15; ++layer) {
    A8_clr();   //A8置0
    chip = 0;     //选0号锁存器
    base = _base + layer;
    //向0-15号锁存器中写数据
    do {
      toggleALEwithA8();      //放通复用锁存器
      busWrite(chip | layer); //向总线送地址
      toggleALEwithA8();      //锁定复用锁存器
      busWrite(_3dBuf[base]);//向总线送数据
      chip += 0x10;           //选下一片锁存器
      base += 15;             //基址+15
    } while (chip != 0);      //当16片锁存器扫描完时，chip上溢归零，结束循环
    A8_set();   //A8置1
    do {
      toggleALE();            //放通复用锁存器
      busWrite(chip | layer); //向总线送地址
      toggleALE();            //锁定复用锁存器
      busWrite(_3dBuf[base]);//向总线送数据
      chip += 0x10;           //选下一片锁存器
      base += 15;             //基址+15
    } while (chip != 0x90);   //写完后9个锁存器，跳出循环

    toggleOE();//扫描完一层，短暂亮灯
    t0 = micros() + REFRESH_US; //计算到时时间
    while (t0 > micros())callback(); //等延时期间，执行回调函数
    toggleOE();//灭灯，进行下一层扫描

  }
}

/*进行一次带着亮度的扫描。
  为减少闪烁感，每个亮度比特的扫描交错排布。*/
inline void fullScan(register void (*callback)()) {
  scan(1125, callback);
  scan(0, callback);
  scan(1125, callback);
  scan(750, callback);
  scan(1125, callback);
  scan(375, callback);
  scan(1125, callback);
  scan(750, callback);
  scan(1125, callback);
  scan(750, callback);
  scan(1125, callback);
  scan(375, callback);
  scan(1125, callback);
  scan(750, callback);
  scan(1125, callback);
}

/*处理双字节控制命令。*/
inline void cmdProc(register uint16_t addr, register uint8_t cmd1) {
  register uint8_t mask = 0x01; //掩码
  mask <<= (cmd1 & 0x07);
  if (cmd1 & 0x08) {
    if (addr & 0x80){
      zeroBuffer();
      return;
    }
    addr |= 0x100;
  }
  if (cmd1 & 0x10) {
    _3dBuf[addr] |= mask;
  } else
    _3dBuf[addr] &= (~mask);
  if (cmd1 & 0x20) {
    _3dBuf[375 + addr] |= mask;
  } else
    _3dBuf[375 + addr] &= (~mask);
  if (cmd1 & 0x40) {
    _3dBuf[750 + addr] |= mask;
  } else
    _3dBuf[750 + addr] &= (~mask);
  if (cmd1 & 0x80) {
    _3dBuf[1125 + addr] |= mask;
  } else
    _3dBuf[1125 + addr] &= (~mask);
}

/*从串口取控制命令的回调函数。*/
static uint8_t si = 0;
void serialFunc() {
  static uint8_t buff[2];
  if (Serial.available() <= 0)return;
  buff[si] = Serial.read();
  ++si;
  if (si >= 2) {
    si = 0;
    cmdProc(buff[0], buff[1]);
  }
}
inline void serialReset() {
  si = 0;
}
