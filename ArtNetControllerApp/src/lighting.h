#ifndef LIGHTING_H_
#define LIGHTING_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum 
{
	DMX_UNIVERSE1,
	DMX_UNIVERSE2,
	DMX_UNIVERSE3,
	DMX_UNIVERSE4,
}tDmxUniverse;

void initLighting(void);
void processLighting(void);
void setRedLeds(uint8_t intensity);
void setGreenLeds(uint8_t intensity);
void setBlueLeds(uint8_t intensity);
void setWarmWhiteLeds(uint8_t intensity);
void writeFrameBuffer(tDmxUniverse dmxUniverse, uint8_t *data, uint16_t length);

#endif /* LIGHTING_H_ */
