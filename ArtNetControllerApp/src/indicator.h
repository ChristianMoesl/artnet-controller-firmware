/**
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */
#ifndef INDICATOR_H_
#define INDICATOR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void initIndicators(void);
void processIndicators(void);
void toggleReceiveLed(void);

#ifdef __cplusplus
}
#endif

#endif /* INDICATOR_H_ */
