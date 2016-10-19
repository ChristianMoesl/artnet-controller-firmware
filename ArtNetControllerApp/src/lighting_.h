/*
 * lighting.h
 *
 * Created: 17.04.2015 18:48:38
 *  Author: Christian
 */ 


#ifndef LIGHTING_H_
#define LIGHTING_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define DMX_UNIVERSE1   0
#define DMX_UNIVERSE2   512
// #define DMX_UNIVERSE3   1024
// #define DMX_UNIVERSE4   1536 

void initLighting(void);
void setRedLeds(uint8_t intensity);
void setGreenLeds(uint8_t intensity);
void setBlueLeds(uint8_t intensity);
bool writeFrameBuffer(uint16_t dmxUniverse, uint8_t *data, uint16_t length, uint16_t timeoutMs);

void processBlinking(void);
void processNightrider(void);
void resetLeds(void);

#endif /* LIGHTING_H_ */
