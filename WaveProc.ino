const static uint16_t barADDR[] PROGMEM = {
  1125, 1155, 1200, 1230, 1275, 1305, 1350, 1380, 1425, 1455
};

#define bitADSC (1<<ADSC)
#define bitADLAR (1<<ADLAR)

/*音频缓冲区，和显存共享。代价是显示方面要砍一级灰度。*/
#define waveBuf ((int8_t*)_3dBuf)

/*FFT点数，按需选取*/
#define FFT_SIZE 128
#define LOG2_FFT_SIZE 7

/*采样周期，单位为微秒*/
#define SAMPLE_US 64

/*修改ADC为快速采样模式*/
inline void myAnalogInit() {
  ADCSRA |= (1 << ADPS2);
  ADCSRA &= ~(1 << ADPS1);
  ADCSRA &= ~(1 << ADPS0);
}

/*简化的8位ADC采样函数*/
inline uint8_t myAnalogRead(register uint8_t port) {
  ADMUX = 0x40 | bitADLAR | port;
  ADCSRA |= bitADSC;
  while (ADCSRA & bitADSC);
  return ADCH;
}

/*音频缓冲区指针*/
static uint8_t pos = 0;

/*采样回调函数*/
void sampleFunc() {
  static uint8_t timeStamp = 0;
  if (pos >= FFT_SIZE)return; //采样已满
  if (timeStamp > micros())return;//采样时间未到
  timeStamp = micros() + SAMPLE_US;//更新下次采样时间
  waveBuf[pos] = (myAnalogRead(1) - 128);
  ++pos;
}

/*对样本做FFT处理，并显示。*/
inline void sampleProc() {
  static int8_t height[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  /*若样本未采满，不予处理*/
  if (pos < FFT_SIZE)return;

  /*虚部访存指针*/
  register int8_t* waveBufI = (int8_t*)(waveBuf + FFT_SIZE);
#define waveBufI8 waveBufI
#define waveBufI16 ((uint16_t*)waveBufI)

  /*虚部清零*/
  for (register uint16_t i = 0; i != FFT_SIZE; ++i)waveBufI8[i] = 0;

  /*送定点FFT函数进行傅里叶变换*/
  fix_fft(waveBuf, waveBufI8, LOG2_FFT_SIZE);

  /*切除直流分量和频谱右半部，进行各频段能量计算*/
  for (register uint16_t i = (FFT_SIZE >> 1) - 1; i != 0; --i) {
    register uint16_t sum = waveBuf[i] * waveBuf[i]; //实部的平方，平方结果必为非负，用无符号16位整数
    sum += waveBufI8[i] * waveBufI8[i];//虚部的平方
    waveBufI16[i] = sum;
  }

  /*准备绘制频谱图，把既有图像向右平移*/
  moveRight();

  /*对数变换、限幅*/
  register uint8_t rs = 0;//右移位数
  register uint8_t p = 1;//缓冲区指针
  for (register uint8_t i = 0; i != 10; ++i)
  {
    register uint16_t temp = 0;
    for (register uint8_t j = 0; j != (1 << rs) ; ++j)
    {
      temp += waveBufI16[p];
      ++p;
    }
    temp >>= rs;
    if (i & 1)++rs;
    if (temp > 10)temp = 10;//限幅
    if (temp < height[i] && height[i] != 0)--height[i];//平滑滤波
    else height[i] = temp;
    bar(i, height[i]);
  }

  /*回开头*/
  pos = 0;
}

/*画面向右平移*/
inline void moveRight() {
  for (register uint8_t i = 0; i != 15; ++i) {
    register uint8_t a;
    for (register uint16_t base = 1485 + i; base > 1139; base -= 15) {
      _3dBuf[base] <<= 2;
      a = _3dBuf[base - 15] >> 6;
      _3dBuf[base] |= a;
    }
    _3dBuf[1125 + i] <<= 2;
    for (register uint16_t base = 1155 + i; base != 1530 + i; base += 75) {
      _3dBuf[base] &= 0xCF;
    }
  }
}

inline void bar(uint8_t id, int8_t h) {
  register uint8_t* base = _3dBuf + pgm_read_word(barADDR + id);
  register uint8_t i = 0;
  for (; h > 0; h -= 2 , ++i) {
    if (i < 4) {
      if (h == 1)base[i + 5] |= (2 << ((id & 1) ? 4 : 0));
      else base[i + 5] |= (3 << ((id & 1) ? 4 : 0));
    }
    if (i > 1) {
      if (h == 1)base[i] |= (2 << ((id & 1) ? 4 : 0));
      else base[i] |= (3 << ((id & 1) ? 4 : 0));
    }
  }
}
