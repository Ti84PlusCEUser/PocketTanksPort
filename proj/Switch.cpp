/*
 * Switch.cpp
 *
 *  Created on: Nov 5, 2023
 *      Author: shyam arumugam, david w chen
 */
#include <ti/devices/msp/msp.h>
#include "inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // PA24-27
  IOMUX->SECCFG.PINCM[PA24INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA25INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA26INDEX] = 0x00040081; // input, no pull
  IOMUX->SECCFG.PINCM[PA27INDEX] = 0x00040081; // input, no pull
}
// return current state
uint32_t Switch_In(void){
  return (GPIOA->DIN31_0 >> 24) & 0x000F; // PA24-27
}
