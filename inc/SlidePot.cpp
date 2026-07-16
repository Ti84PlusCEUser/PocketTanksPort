/* SlidePot.cpp
 * David Chen and Shyam Arumugam
 * Modified: put the date here
 * 12-bit ADC input on ADC1 channel 5, PB18
 */
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"
#include "../inc/SlidePot.h"
#define ADCVREF_VDDA 0x000
#define ADCVREF_INT  0x200
volatile uint32_t ADCData = 0;       // Raw ADC measurement
volatile uint32_t ADCSemaphore = 0;  // Data-ready flag

void SlidePot::Init(void){
// write code to initialize ADC1 channel 5, PB18
// Your measurement will be connected to PB18
// 12-bit mode, 0 to 3.3V, right justified
// software trigger, no averaging
  
  // write this
    // ADC1 Channel 5 (PB18) initialization
    ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003;     // Reset ADC
    ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;      // Power enable
    Clock_Delay(24);                            // Wait 24 cycles
    ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000;
    ADC1->ULLMEM.CLKFREQ = 7;                   // 40-48 MHz clock
    ADC1->ULLMEM.CTL0 = 0x03010000;             // 12-bit mode, CLK/8
    ADC1->ULLMEM.CTL1 = 0x00000000;
    ADC1->ULLMEM.CTL2 = 0x00000000;
    ADC1->ULLMEM.MEMCTL[0] = 5;                    // Channel 5
    ADC1->ULLMEM.SCOMP0 = 0;
    ADC1->ULLMEM.CPU_INT.IMASK = 0;
}

uint32_t SlidePot::In(void){
  // write code to sample ADC1 channel 5, PB18 once
  // return digital result (0 to 4095)
  ADC1->ULLMEM.CTL0 |= 0x00000001;       // Set EN=1 (bit 0)
  ADC1->ULLMEM.CTL1 |= 0x00000100;       // Set START=1 (bit 8)
  uint32_t volatile delay=ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS&0x01)==0x01) {}
  return ADC1->ULLMEM.MEMRES[0];
}


// constructor, invoked on creation of class
// m and b are linear calibration coefficents
SlidePot::SlidePot(uint32_t m, uint32_t b) {
    this->slope = m;        // Initialize slope 
    this->offset = b;       // Initialize offset 
    this->distance = 0;     // Initialize distance to 0
    this->flag = 0;         // Clear semaphore flag 
}


void SlidePot::Save(uint32_t n) {
    this->data = n;                 
    this->distance = Convert(n);    
    this->flag = 1;                 
}

uint32_t SlidePot::Convert(uint32_t n) {
  return ((1999 * n) >> 12) + 1;
}


// do not use this function in final lab solution
// it is added just to show you how SLOW floating point in on a Cortex M0+
float SlidePot::FloatConvert(uint32_t input){
  return 0.00048828125*input -0.0001812345;
}

void SlidePot::Sync(void) {
    while(!ADCSemaphore) {} // Wait for new data
    ADCSemaphore = 0;       // Clear flag
}


uint32_t SlidePot::Distance(void) {
    return this->distance; // Return calibrated value (0-2000 = 0.0-2.000cm)
}