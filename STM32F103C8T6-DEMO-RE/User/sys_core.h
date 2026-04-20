#ifndef __SYS_CORE_H
#define __SYS_CORE_H

#include "sys_config.h"

#ifdef __cplusplus
	extern "C" {
#endif

void sys_core_init(void);

#if SYS_EN_RTC
	void sys_sync_time(void);
#endif /* SYS_EN_RTC */

#ifdef __cplusplus
}
#endif

#endif  /* __SYS_CORE_H */