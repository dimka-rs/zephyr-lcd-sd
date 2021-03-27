
# Pinout

|Pin|Usage|
|---|-----|
|22|SD_SS|
|23|SD_DI|
|24|SD_DO|
|25|SD_SCK|
|||
|19|LCD_D0|
|20|LCD_D1|
|13|LCD_D2|
|14|LCD_D3|
|15|LCD_D4|
|16|LCD_D5|
|17|LCD_D6|
|18|LCD_D7|
|30|LCD_RST|
|29|LCD_CS AIN5|
|28|LCD_RS AIN4|
|04|LCD_WR AIN2|
|03|LCD_RD AIN1|

# LCD (Thanks MCUFRIEND_kbv Arduino library)

```
Testing : (A1, D7) = 40
Testing : (A2, D6) = 26
ID = 0x9325

*** COPY-PASTE from Serial Terminal:
const int XP=6,XM=A2,YP=A1,YM=7; //240x320 ID=0x9325
const int TS_LEFT=155,TS_RT=913,TS_TOP=961,TS_BOT=189;

PORTRAIT  CALIBRATION     240 x 320
x = map(p.x, LEFT=155, RT=913, 0, 240)
y = map(p.y, TOP=961, BOT=189, 0, 320)

LANDSCAPE CALIBRATION     320 x 240
x = map(p.y, LEFT=961, RT=189, 0, 320)
y = map(p.x, TOP=913, BOT=155, 0, 240)
```