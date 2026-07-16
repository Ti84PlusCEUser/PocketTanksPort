// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Shyam Arumugam, David Chen

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "inc/ST7735.h"
#include "inc/Clock.h"
#include "inc/LaunchPad.h"
#include "inc/TExaS.h"
#include "inc/Timer.h"
#include "inc/SlidePot.h"
#include "inc/DAC5.h"
#include "proj/SmallFont.h"
#include "proj/LED.h"
#include "proj/Switch.h"
#include "proj/Sound.h"
#include "images/images.h"
#include <math.h>
extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);

#define LEFT_TANK_MIN_SAFE_ANGLE 31     // min angle to clear mountain
#define RIGHT_TANK_MIN_SAFE_ANGLE 56    // min angle to clear mountain
#define TUNNEL_PREVENTION_DELAY 20      // number of frames before stopping
// --- Constants for projectile
#define MOUNTAIN_ROW_MIN 104
#define MOUNTAIN_ROW_MAX 107
#define MOUNTAIN_X_MIN 0
#define MOUNTAIN_X_MAX 81

static uint32_t swinput = 0, spinput = 0, shotPower = 0, shotAngle = 0, lastAngleRead = 0;
uint32_t leftScore = 0, rightScore = 0, leftHP = 3, rightHP = 3;

// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

SlidePot Sensor(1500,0); // copy calibration from Lab 7

uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="Down to Play";
const char Language_Spanish[]="Down para Juega";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

bool CheckCollision(int projX, int projY, bool isLeftTankTurn) {
  int tx = isLeftTankTurn ? 50 : 37;
  int ty = isLeftTankTurn ? 50 : 145;
  int tankW = 10;
  int tankH = 17;

  // Tank is drawn with y as bottom
  int tankTop = ty - tankH;
  int tankBottom = ty;
  int tankLeft = tx;
  int tankRight = tx + tankW;

  return (projX >= tankLeft && projX <= tankRight &&
          projY >= tankTop  && projY <= tankBottom);
}

void FireProjectile(bool isLeftTank) {
  ST7735_SetRotation(0);
  const float PI = 3.14159;

  // --------- Setup Physics ---------
  float angleDeg = isLeftTank ? 180.0f - (float)shotAngle : (float)shotAngle;
  float angleRad = angleDeg * PI / 180.0f;

  float powerNorm = ((float)shotPower) / 1000.0f;
  float v0 = 3.0f + 5.0f * powerNorm;

  float vx = v0 * cos(angleRad);
  float vy = v0 * sin(angleRad);
  float gravity = -0.25f;

  float x_pos = 0.0f;
  float y_pos = 0.0f;

  // --------- Initial screen position ---------
  float screen_x = isLeftTank ? (37.0f + 10.0f) : (50.0f + 10.0f);
  float screen_y = isLeftTank ? (128.0f + 4.0f) : (33.0f + 17.0f);

  // --------- Charge animation ---------
  if (isLeftTank) {
    ST7735_DrawBitmap(37, 145, LeftFacingTankChargeUp, 10, 17);
  } else {
    ST7735_DrawBitmap(50, 50, RightFacingTankChargeUp, 10, 17);
  }
  Clock_Delay1ms(2000);
  Sound_Shoot();

  // --------- Launch loop ---------
  while (1) {
  int drawX = (int)(screen_x + y_pos);
  int drawY = (int)(screen_y + x_pos);

  if (drawX < -3 || drawX > 130 || drawY > 159) break;

  // ---------- Redraw background and tanks ----------
  ST7735_DrawBitmap(0, 159, Background, 128, 160);
  ST7735_DrawBitmap(37, 145, LeftFacingTank, 10, 17);
  ST7735_DrawBitmap(50, 50, RightFacingTank, 10, 17);

  // ---------- Draw projectile ----------
  ST7735_DrawBitmap(drawX, drawY, TankBullet, 3, 3);
  Clock_Delay1ms(10);

  // ---------- Stop if projectile hits mountain ridge ----------
  if ((drawY >= 104 && drawY <= 107) &&
      (drawX >= 0  && drawX <= 81)) {
    break; // hit mountain region
  }

  // ---------- Tank collision ----------
  if (CheckCollision(drawX, drawY, isLeftTank)) {
    if (!isLeftTank) { // Right tank hit
      Sound_Explosion();
      rightHP--;
      leftScore += 10;
      if (rightHP == 0) {
        ST7735_DrawBitmap(37, 145, LeftFacingTankDefeated, 15, 17);
      }
    } else { // Left tank hit
      Sound_Explosion();
      leftHP--;
      rightScore += 10;
      if (leftHP == 0) {
        ST7735_DrawBitmap(50, 50, RightFacingTankDefeated, 15, 17);
      }
    }
    Clock_Delay1ms(1000); // Optional pause to show defeated sprite
    break;
  }

  // ---------- Physics update ----------
  x_pos += vx;
  vy += gravity;
  y_pos += vy;
  }

  // --------- Restore tank sprite ---------
  ST7735_DrawBitmap(0, 159, Background, 128, 160);

if (rightHP > 0) {
  ST7735_DrawBitmap(37, 145, LeftFacingTank, 10, 17);
} else {
  ST7735_DrawBitmap(37, 145, LeftFacingTankDefeated, 15, 17);
}

if (leftHP > 0) {
  ST7735_DrawBitmap(50, 50, RightFacingTank, 10, 17);
} else {
  ST7735_DrawBitmap(50, 50, RightFacingTankDefeated, 15, 17);
}

  ST7735_SetRotation(1); // restore UI rotation
}
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
    TimerG12_Init();
    TimerG12_IntArm(2666667, 3);
  
  // initialize all data structures
  __enable_irq();
  ST7735_SetRotation(1); // or 2, 3 depending on the orientation you want
printf("Down for SPANISH");
ST7735_SetCursor(0, 1);
ST7735_OutString((char *)"Left for ENGLISH");
bool langflag = true;
while (langflag){
  if(swinput & 0x04){
    myLanguage=English;
    langflag = false;      
  } else if(swinput & 0x02){
      myLanguage=Spanish;
      langflag = false;
  } 
}
ST7735_SetCursor(0, 2);
ST7735_OutString((char *)Phrases[0][myLanguage]);
for(uint32_t t=500;t>0;t=t-5){
    Clock_Delay1ms(10);              // delay 50 msec
}
ST7735_SetRotation(0); // or 2, 3 depending on the orientation you want
ST7735_FillScreen(ST7735_BLACK);
ST7735_DrawBitmap(0,159, HomeScreen, 127,159); // player ship bottom

for(uint32_t t=500;t>0;t=t-5){
    Clock_Delay1ms(20);              // delay 50 msec
}

ST7735_SetRotation(1); // or 2, 3 depending on the orientation you want
ST7735_SetCursor(5, 10);
ST7735_OutString((char *)Phrases[2][myLanguage]);

ST7735_SetRotation(0); // or 2, 3 depending on the orientation you want
bool startflag = true;
while (startflag){     
  if(swinput & 0x02){
      ST7735_FillScreen(ST7735_BLACK);
      ST7735_DrawBitmap(0,159, Background, 128,160); // player ship bottom
      startflag = false;
  } 
}

  ST7735_DrawBitmap(50,50, RightFacingTank, 10,17); // player ship bottom
  ST7735_DrawBitmap(37,145, LeftFacingTank, 10,17); // player ship bottom

  ST7735_SetRotation(1); // or 2, 3 depending on the orientation you want
  for (int round = 0; round < 5; round++) {
  // ======== RIGHT (Left Facing) TANK TURN ========
  ST7735_SetCursor(0, 12);
  if (myLanguage == English) {
    ST7735_OutString((char *)"Right's Turn");
  } else {
    ST7735_OutString((char *)"La Derecha Juega");
  }

  bool fireflag = true;
  while (fireflag) {
    // Display power
    ST7735_SetCursor(0, 11);
    if (myLanguage == English) {
      ST7735_OutString((char *)"Power:");
    } else {
      ST7735_OutString((char *)"Potencia:");
    }
    ST7735_OutUDec(shotPower);

   // ----------- Display angle label -----------
ST7735_SetCursor(0, 10);
if (myLanguage == English) {
  ST7735_OutString((char *)"Angle:");
} else {
  ST7735_OutString((char *)"Direcci\xA2n:");
}

// ----------- Read and update angle -----------
uint32_t currentAngleRead = (swinput & 0x04);
if ((lastAngleRead == 0) && (currentAngleRead != 0)) {
  shotAngle = (shotAngle + 5) % 185; // cycles from 0 to 180
}
lastAngleRead = currentAngleRead;

// ----------- Clear old angle and redraw new one -----------
ST7735_SetCursor(10, 10);
ST7735_OutString((char *)"    "); // clear area
ST7735_SetCursor(10, 10);
ST7735_OutUDec(shotAngle);

    if (swinput & 0x02) {
      fireflag = false;
    }
  }




  ST7735_SetCursor(0, 12);
  ST7735_OutString((char*)"                  "); // clear previous angle
  ST7735_SetCursor(0, 12);
  if (myLanguage == English) {
      ST7735_OutString((char *)"Firing Now");
    } else {
      ST7735_OutString((char *)"Disparando Ya");
    }
    // FireProjectile(true); // Call this later once implemented
  FireProjectile(true);
  Clock_Delay1ms(500);
  ST7735_SetCursor(0, 12);
  ST7735_OutString((char*)"                  "); // clear previous angle
  ST7735_SetCursor(0, 12);
  if (leftHP == 0 || rightHP == 0) {
        break; // someone died early
      }

  
  // --------- LEFT (Right Facing) TANK TURN ------------
  ST7735_SetCursor(0, 12);
  if (myLanguage == English) {
    ST7735_OutString((char *)"Left's Turn");
  } else {
    ST7735_OutString((char *)"La Izquierda Juega");
  }

  fireflag = true;
  while (fireflag) {
    // Display power
    ST7735_SetCursor(0, 11);
    if (myLanguage == English) {
      ST7735_OutString((char *)"Power:");
    } else {
      ST7735_OutString((char *)"Potencia:");
    }
    ST7735_OutUDec(shotPower);

    // ----------- Display angle label -----------
ST7735_SetCursor(0, 10);
if (myLanguage == English) {
  ST7735_OutString((char *)"Angle:");
} else {
  ST7735_OutString((char *)"Direcci\xA2n:");
}

// ----------- Read and update angle -----------
uint32_t currentAngleRead = (swinput & 0x04);
if ((lastAngleRead == 0) && (currentAngleRead != 0)) {
  shotAngle = (shotAngle + 5) % 185; // cycles from 0 to 180
}
lastAngleRead = currentAngleRead;

// ----------- Clear old angle and redraw new one -----------
ST7735_SetCursor(10, 10);
ST7735_OutString((char *)"    "); // clear area
ST7735_SetCursor(10, 10);
ST7735_OutUDec(shotAngle);
    if (swinput & 0x02) {
      fireflag = false;
    }
  }


  ST7735_SetCursor(0, 12);
  ST7735_OutString((char*)"                  "); // clear previous angle
  ST7735_SetCursor(0, 12);
  if (myLanguage == English) {
      ST7735_OutString((char *)"Firing Now");
    } else {
      ST7735_OutString((char *)"Disparando Ya");
    }
  // FireProjectile(false); // Call this later once implemented
  FireProjectile(false);
  Clock_Delay1ms(500);
  ST7735_SetCursor(0, 12);
  ST7735_OutString((char*)"                  "); // clear previous angle
  ST7735_SetCursor(0, 12);
  if (leftHP == 0 || rightHP == 0) {
        break; // someone died early
      }
}
    int leftScore = (3 - rightHP) * 10;
  int rightScore = (3 - leftHP) * 10;
  int leftTankX = 37;
int leftTankY = 145;
int rightTankX = 50;
int rightTankY = 50;

// Correct win/loss detection
bool leftLost = (leftHP == 0) || (leftScore < rightScore);
bool rightLost = (rightHP == 0) || (rightScore < leftScore);

// Ensure draw still shows animation
if (!leftLost && !rightLost && leftScore == rightScore) {
  leftLost = false;
  rightLost = false;
}

// Explosion effect (rises down)
ST7735_SetRotation(0);
for (int i = 0; i < 30; i++) {
  ST7735_DrawBitmap(0,159, Background, 128,160); // player ship bottom
  if (leftLost) {
    ST7735_DrawBitmap(37,145, LeftFacingTank, 10,17); // player ship bottom
    ST7735_DrawBitmap(rightTankX - i * 2, rightTankY, RightFacingTankDefeated, 15, 17);
  }
  if (rightLost) {
    ST7735_DrawBitmap(50,50, RightFacingTank, 10,17); // player ship bottom
    ST7735_DrawBitmap(leftTankX - i * 2, leftTankY, LeftFacingTankDefeated, 15, 17);
  }
  Clock_Delay1ms(50);
}

// Victory slide-away effect
for (int i = 0; i < 15; i++) {
  ST7735_DrawBitmap(0,159, Background, 128,160); // player ship bottom
  if (!leftLost) {
    ST7735_DrawBitmap(rightTankX, rightTankY + i, RightFacingTank, 10, 17);
  }
  if (!rightLost) {
    ST7735_DrawBitmap(leftTankX, leftTankY + i, LeftFacingTank, 10, 17);
  }
  Clock_Delay1ms(100);
}

  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(0,159, HomeScreen, 127,159); // player ship bottom
ST7735_SetRotation(1);
  ST7735_SetCursor(1, 4);
  if (myLanguage == English) {
    ST7735_OutString((char*)"Game Over!");
  } else {
    ST7735_OutString((char*)"\xADJuego Terminado!");
  }

  ST7735_SetCursor(1, 6);
  if (myLanguage == English) {
    ST7735_OutString((char*)"Left Score: ");
  } else {
    ST7735_OutString((char*)"Puntaje Izq: ");
  }
  ST7735_OutUDec(leftScore);

  ST7735_SetCursor(1, 8);
  if (myLanguage == English) {
    ST7735_OutString((char*)"Right Score: ");
  } else {
    ST7735_OutString((char*)"Puntaje Der: ");
  }
  ST7735_OutUDec(rightScore);

  ST7735_SetCursor(1, 10);
  if (leftScore > rightScore) {
    if (myLanguage == English) {
      ST7735_OutString((char*)"Left Wins!");
    } else {
      ST7735_OutString((char*)"\xADGana la Izquierda!");
    }
  } else if (rightScore > leftScore) {
    if (myLanguage == English) {
      ST7735_OutString((char*)"Right Wins!");
    } else {
      ST7735_OutString((char*)"\xADGana la Derecha!");
    }
  } else {
    if (myLanguage == English) {
      ST7735_OutString((char*)"It's a draw!");
    } else {
      ST7735_OutString((char*)"Empate.");
    }
  }

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    swinput = Switch_In(); // all buttons taken in
    spinput = Sensor.Convert(Sensor.In());
    shotPower = (2000 - spinput)>>1;
  }
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
