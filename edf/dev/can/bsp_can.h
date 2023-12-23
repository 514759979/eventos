
#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

// include ---------------------------------------------------------------------
#include "dev_can.h"

// 注
// CAN设备是板载的，只能进行初始化，没有选择，因此不必对各个CAN设备单独初始化。

// API -------------------------------------------------------------------------
void bsp_can_init(void);

#endif