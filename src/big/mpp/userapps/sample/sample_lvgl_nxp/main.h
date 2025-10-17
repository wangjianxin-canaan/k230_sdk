#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

int uart_init(void);
int lv_port_uninit_k230(void);
int lv_port_init_k230(k_connector_type connector_type);
extern unsigned width , height ;

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
