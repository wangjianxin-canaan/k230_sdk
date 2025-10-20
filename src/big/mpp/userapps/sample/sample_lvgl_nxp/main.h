#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "mpi_connector_api.h"
#include <src/core/lv_global.h>
int uart_init(void);
int lv_port_uninit_k230(void);
int lv_port_init_k230(k_connector_type connector_type);
extern unsigned width , height ;
extern bool flag_running;
#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
