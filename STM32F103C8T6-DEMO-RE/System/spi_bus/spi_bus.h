#ifndef __SPI_BUS_H
#define __SPI_BUS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	SPI_BUS_OK = 0,
	SPI_BUS_ERR_BUSY,
	SPI_BUS_ERR_TIMEOUT,
	SPI_BUS_ERR_INVALID_PARAM
} spi_bus_status_t;

// SPI 模式定义 (CPOL, CPHA)
typedef enum {
    SPI_MODE_0 = 0, // CPOL=0, CPHA=0
    SPI_MODE_1 = 1, // CPOL=0, CPHA=1
    SPI_MODE_2 = 2, // CPOL=1, CPHA=0
    SPI_MODE_3 = 3  // CPOL=1, CPHA=1
} spi_bus_mode_t;

typedef struct {
	spi_bus_status_t (*init)(void *user_data);
	spi_bus_status_t (*write_byte)(void *user_data, uint8_t byte);
	spi_bus_status_t (*read_byte)(void *user_data, uint8_t *byte);
	spi_bus_status_t (*switch_byte)(void *user_data, uint8_t tx, uint8_t *rx);
	
	spi_bus_status_t (*write_bytes)(void *user_data, const uint8_t *data, uint16_t len);
	spi_bus_status_t (*read_bytes)(void *user_data, uint8_t *data, uint16_t len);
	spi_bus_status_t (*switch_bytes)(void *user_data, const uint8_t *tx, uint8_t *rx, uint16_t len);
	
	// 动态配置 (SPI 特性：不同设备可能需要不同速率和模式)
    spi_bus_status_t (*set_speed)(void *user_data, uint32_t speed_hz);
    spi_bus_status_t (*set_mode)(void *user_data, spi_bus_mode_t mode);
} spi_bus_ops_t;

typedef struct {
    void *user_data; 
	spi_bus_ops_t *ops;
} spi_bus_handle_t;

typedef struct {
	spi_bus_status_t (*init)(void *user_data);
	spi_bus_status_t (*low)(void *user_data);
	spi_bus_status_t (*high)(void *user_data);
} spi_cs_ops_t;

typedef struct {
	void *user_data; 
	spi_cs_ops_t *ops;
} spi_cs_handle_t;

typedef struct {
	spi_bus_handle_t *bus;		// 关联的总线
	spi_cs_handle_t *cs;		// 关联的片选控制器
} spi_dev_handle_t;

spi_bus_status_t spi_cs_low(spi_dev_handle_t *handle);
spi_bus_status_t spi_cs_high(spi_dev_handle_t *handle);

spi_bus_status_t spi_master_transmit(spi_dev_handle_t *handle, const uint8_t *data, uint16_t len);
spi_bus_status_t spi_master_receive(spi_dev_handle_t *handle, uint8_t *data, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif  /* __SPI_BUS_H */