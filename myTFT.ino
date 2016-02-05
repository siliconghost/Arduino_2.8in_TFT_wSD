#include "TouchScreen.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Pins for the LCD Shield
#define YP A3 // must be analog
#define XM A2 // must be analog
#define YM 9  // digital or analog pin
#define XP 8  // digital or analog pin

#define MINPRESSURE 1
#define MAXPRESSURE 1000

// Calibration mins and max for raw data when touching edges of screen
#define TS_MINX 210
#define TS_MINY 210
#define TS_MAXX 915
#define TS_MAXY 910


// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

const char* touchMeStr = "Touch Me / Press Button";

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int touchCnt = 0;
int buttonState = 0;
long int timeCnt;
char drawChars[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'};
const int SCREEN_TIMEOUT = 10000;


void setup() {
  // put your setup code here, to run once:

  tft.reset();

  uint16_t identifier = tft.readID();

  tft.begin(identifier);

  tft.fillScreen(BLACK);
  tft.setRotation(1);
  tft.setCursor(30,100);
  tft.setTextColor(RED); 
  tft.setTextSize(2);
  tft.println("LCD driver chip: ");
  tft.setCursor(100, 150);
  tft.setTextColor(BLUE);
  tft.println(identifier, HEX);

  delay(3000);

  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW);

 tft.setCursor(0, 0);
 tft.println(touchMeStr);

 timeCnt = millis();
}


void loop() {
  // put your main code here, to run repeatedly:

  // a point object holds x y and z coordinates
  TSPoint p = ts.getPoint();

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (millis() > (timeCnt + SCREEN_TIMEOUT))
  {
    RunScreenSaver();
  } else {
    drawButtons();
  }

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // reset the screen
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println(touchMeStr);

    long colorPressure = (p.z * 65535) / 1000; //color changes dependent on pressure

    colorPressure = YELLOW; // just to make it easier to see when testing

    if (touchCnt == 10)
    {
      touchCnt = 0;
    }

    // May need to adjust if cordinates are being reversed due to screen rotation
    int YY = tft.height() - (map(p.x, TS_MINX, TS_MAXX, 0, tft.height()));
    int XX = tft.width() - (map(p.y, TS_MINY, TS_MAXY, 0, tft.width()));

    // determine if a button has been pressed
    if (XX > 295 && YY < 30) // first button region 30 up
      buttonState = 0;
    if ((XX >= 295 && YY >=30) && (XX >= 295 && YY < 60)) // second button region 30 thru 59
      buttonState = 1;
    if ((XX >= 295 && YY >=60) && (XX >= 295 && YY < 100)) // second button region 60 thru 99
      buttonState = 2;
    if ((XX > 295 && YY >=100) && (XX >= 295 && YY < 120)) // second button region 100 thru 119
      buttonState = 3;
    if ((XX > 295 && YY >= 120) && (XX >= 295 && YY < 155)) // second button region 120 thru 154
      buttonState = 4;
    if ((XX >= 295 && YY >= 155) && (XX >= 295 && YY < 200)) // second button region 155 thru 200
      buttonState = 5;

    // button pressed
    switch (buttonState)
    {
      case 0:
        tft.fillCircle(305, 20, 10, RED);
        tft.drawCircle(305, 20, 10, GREEN);
        tft.drawCircle(XX, YY, p.z / 10, colorPressure);
        break;

      case 1:
        tft.fillTriangle(295, 60, 305, 40, 315, 60, RED);
        tft.drawTriangle(XX, YY, XX + 50, YY + 50, YY - 50, XX - 50, colorPressure);
        break;

      case 2:
        tft.fillRect(295, 75, 20, 15, RED);
        tft.drawRect(XX, YY, 100, 50, colorPressure);
        break;

      case 3:
        tft.drawCircle(308, 110, 10, RED);
        tft.drawCircle(299, 110, 10, RED);
        DrawDesign(XX, YY, 3, 25);
        break;

      case 4:
        tft.drawTriangle(295, 155, 305, 125, 315, 140, RED);
        tft.drawTriangle(300, 155, 310, 125, 320, 140, RED);
        TriangleDesign(XX, YY, 5);
        break;

      case 5:
        tft.drawChar(295, 160, 'a', RED, RED, 3);
        tft.drawChar(XX, YY, drawChars[touchCnt], colorPressure, colorPressure, 6);
        break;
    }

    // Prints out the raw and transposed data which is useful for troubleshooting

    tft.setTextSize(1);
    tft.setTextColor(colorPressure); // converts the pressure reading to a color

    tft.print("Raw X = ");
    tft.print(p.x);

    tft.print(" RAW Y = ");
    tft.println(p.y);

    tft.print(" Pressure = ");
    tft.println(p.z);

    tft.print("ScreenX: ");
    tft.println(XX);
    tft.print("ScreenY: ");
    tft.println(YY);

    tft.print("LCD Driver: ");
    tft.println(tft.readID(), HEX);

    touchCnt = touchCnt + 1;
    timeCnt = millis();   
    }
    
}

void drawButtons()
{
  // Setup Buttons
  // circle button unpressed
  if (buttonState == 0)
    tft.fillCircle(305, 20, 10, RED);
  else
    tft.fillCircle(305, 20, 10, GREEN);

  // triangle button
  if (buttonState == 1)
  {
    tft.fillTriangle(295, 60, 305, 40, 315, 60, RED);
  } else {
    tft.fillTriangle(295, 60, 305, 40, 315, 60, GREEN);
  }

  // rectangle button
  if (buttonState == 2)
  {
    tft.fillRect(295, 75, 20, 15, RED);
  } else {
    tft.fillRect(295, 75, 20, 15, GREEN);
  }

  // circle button
  if (buttonState == 3)
  {
    tft.drawCircle(308, 110, 10, RED);
    tft.drawCircle(299, 110, 10, RED);
  } else {
    tft.drawCircle(308, 110, 10, GREEN);
    tft.drawCircle(299, 110, 10, GREEN);
  }

  // triangle button
  if (buttonState == 4)
  {
    tft.drawTriangle(295, 155, 305, 125, 315, 140, RED);
    tft.drawTriangle(300, 155, 310, 125, 320, 140, RED);
  } else {
    tft.drawTriangle(295, 155, 305, 125, 315, 140, GREEN);
    tft.drawTriangle(300, 155, 310, 125, 320, 140, GREEN);
  }

  // character draw button
  if (buttonState == 5)
    tft.drawChar(295, 160, 'a', RED, RED, 3);
  else
    tft.drawChar(295, 160, 'a', GREEN, GREEN, 3);
}

  
void RunScreenSaver()
{
  randomSeed(millis() / 100);

  int x = random(20, 300);
  int y = random(20, 220);

  tft.fillScreen(BLACK);
  tft.setCursor(x - 50, y + 50);
  //tft.setTextSize(x / 100);
  tft.setTextSize(1);
  tft.print("myTFT Screen Saver");
  delay(1000);

}

void TriangleDesign(int XX, int YY, int mp)
{
  for (int i=0; i<5; i++)
  {
    tft.drawTriangle(XX, YY, XX + (mp * i), YY + (mp * i), YY - (mp * i), XX - (mp * i), RED);
  }
}

void DrawDesign(int XX, int YY, int mp, int rad)
{
  for (int i=0; i<5; i++)
  {
    tft.drawCircle(XX + (i * mp), YY - (i * mp), rad, RED);
    tft.drawCircle(XX + (i * mp), YY - (i * mp), rad, GREEN);
    tft.drawCircle(XX + (i * mp), YY - (i * mp), rad, BLUE);
    tft.drawCircle(XX + (i * mp), YY - (i * mp), rad, YELLOW);
  }
}

void CircleMove(int speed)
{
  for (int i = 0; i <320; i++)
  {
    tft.drawCircle(i + speed, 120, 30, WHITE);
    tft.fillScreen(BLACK);
  }
}


