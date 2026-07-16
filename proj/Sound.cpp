// Sound.cpp
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// Shyam Arumugam
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "proj/Sound.h"
#include "sounds/sounds.h"
#include "inc/DAC5.h"
#include "inc/Timer.h"

static uint32_t sinindex = 0;
static const uint8_t *soundarr = nullptr;
static uint32_t maxindex = 0;
static bool loop = false;
void SysTick_IntArm(uint32_t period, uint32_t priority){
  SysTick->LOAD = period-1;
  sinindex=0;
  SysTick->VAL = 0;
}
void Sound_Init(void){
  SysTick->CTRL = 0;
  SysTick->LOAD = 0; 
  SysTick->VAL = 0;
  SCB->SHP[1] = (SCB->SHP[1] & ~(0xC0000000)) | (0x00 << 30);
  SysTick->CTRL = 0x00000007; 
  DAC5_Init();
  sinindex = 0;
}
extern "C" void SysTick_Handler(void);
void SysTick_Handler(void){ 
  DAC5_Out(soundarr[sinindex]);
  sinindex++;
  if (sinindex == maxindex && !loop) {
    SysTick->LOAD = 0;
  } else {
    sinindex = sinindex % maxindex;
  }
}
//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count, bool loopset = false){
// write this
  soundarr = pt;
  maxindex = count;
  sinindex = 0;
  loop = loopset;
  SysTick->LOAD = 7256-1;
}
void Sound_Shoot(void){
  Sound_Start(shoot, 4080);
}
void Sound_Killed(void){
  Sound_Start(invaderkilled, 3377);  
}
void Sound_Explosion(void){
  Sound_Start(explosion, 2000); 
}
void Sound_Fastinvader1(void){
  Sound_Start(highpitch, 1802);
}
void Sound_Fastinvader2(void){
}
void Sound_Fastinvader3(void){
}
void Sound_Fastinvader4(void){
}
void Sound_Highpitch(void){
}
