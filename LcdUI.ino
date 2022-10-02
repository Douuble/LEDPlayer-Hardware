/*管理ADC键盘。键被按下之后第一次调用本函数，返回按键编号，
  在松开按键之前，返回的都是0xFF。没有键被按下时也返回0xFF。*/
inline uint8_t KeyManager() {
  static uint8_t KeyStat = 0;
  static uint32_t timeStamp = 5000;
  if (timeStamp > millis())return 0xFF;
  else timeStamp = millis() + 100;      //软件防抖动
  register uint8_t a = myAnalogRead(2);
  for (register uint8_t i = 0;; ++i) {
    if (pgm_read_byte_near(KeyVal + i) >= a) {
      if (KeyStat & (1 << i))return 0xFF;
      else {
        KeyStat = (1 << i);
        return i;
      }
    } else KeyStat &= ~(1 << i);
  }
}

/*绘制菜单*/
inline void drawMenu(const uint8_t* p) {
  register uint8_t ln = 0;
  register int8_t i = -1;
  for (; pgm_read_byte_near(p) != 0xFE; ++p) {
    register uint8_t buf[8];
    register uint8_t* ptr = fontData + (pgm_read_byte_near(p) << 3);
    for (register uint8_t j = 0; j != 8; ++j)buf[j] = pgm_read_byte_near(ptr + j);
    lcd.createChar(++i, buf);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  for (++p; pgm_read_byte_near(p) != 0xFE; ++p) {
    if (pgm_read_byte_near(p) == '\n')lcd.setCursor(0, ++ln);
    else lcd.write(pgm_read_byte_near(p));
  }
}

/*LCD屏幕初始化*/
inline void LcdUIInit() {
  lcd.init();
  lcd.backlight();
}
