#ifndef __W25QX_H
#define __W25QX_H

#include "stdint.h"
#include "spi_bus.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    W25QX_OK = 0,
	W25QX_ERR_BUSY,
	W25QX_ERR_CAL,
	W25QX_ERR_SPI_ERR,
	W25QX_ERR_INVALID_PARAM
} W25QX_Status_t;

void W25QX_Init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __W25QX_H */