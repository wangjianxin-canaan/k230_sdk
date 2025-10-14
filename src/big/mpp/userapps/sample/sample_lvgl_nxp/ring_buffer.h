#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// 定义缓冲区大小 (通常选择 2 的幂次方，便于使用位运算优化 % 运算符，但本例中使用标准 % 运算符)
#define BUFFER_SIZE 64

// 环形缓冲区结构体定义
typedef struct {
    uint8_t buffer[BUFFER_SIZE]; // 存储数据的数组
    size_t head;                 // 写入指针/索引 (生产者)
    size_t tail;                 // 读取指针/索引 (消费者)
    size_t count;                // 缓冲区中当前元素的数量
} RingBuffer;

// 初始化环形缓冲区
void ring_buffer_init(RingBuffer *cb);

// 写入数据到缓冲区 (发送/生产)
// data: 要写入的数据
// 返回值: true 成功, false 失败 (缓冲区已满)
bool ring_buffer_write(RingBuffer *cb, uint8_t data);

// 从缓冲区读取数据 (接收/消费)
// data: 用于存储读取到的数据的指针
// 返回值: true 成功, false 失败 (缓冲区为空)
bool ring_buffer_read(RingBuffer *cb, uint8_t *data);

// 检查缓冲区是否为空
bool ring_buffer_is_empty(const RingBuffer *cb);

// 检查缓冲区是否已满
bool ring_buffer_is_full(const RingBuffer *cb);
extern  RingBuffer uart_ring_buffer;
typedef void * (*thread_func_t)(void *);
void create_detach_thread(thread_func_t func);
void * proc_uart_data(void *arg);
#endif // RING_BUFFER_H
