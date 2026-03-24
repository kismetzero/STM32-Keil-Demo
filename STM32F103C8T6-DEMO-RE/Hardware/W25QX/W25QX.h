#ifndef __W25QX_H
#define __W25QX_H

#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
    W25QX_STATUS_OK = 0,
	W25QX_STATUS_ERR_BUSY,
	W25QX_STATUS_ERR_CAL,
	W25QX_STATUS_ERR_SPI_ERR,
	W25QX_STATUS_ERR_INVALID_PARAM
} W25QX_Status_t;

// 定义 W25QX 设备句柄结构体
typedef struct {
	void *hspi;
} W25QX_Handle_t;

W25QX_Status_t W25QX_Init(W25QX_Handle_t *dev, void *hspi);
W25QX_Status_t W25QX_Reset(W25QX_Handle_t *dev);
W25QX_Status_t W25QX_ReadID(W25QX_Handle_t *dev);
W25QX_Status_t W25QX_ReadData(W25QX_Handle_t *dev);
W25QX_Status_t W25QX_WriteData(W25QX_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif  /* __W25QX_H */