#include "ring_buffer.h"
RingBuffer uart_ring_buffer_rcv;
//RingBuffer uart_ring_buffer_send;
// 初始化环形缓冲区
void ring_buffer_init(RingBuffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    // 数组内容不必清零，因为只有 head 到 tail 之间的数据是有效的
}

// 检查缓冲区是否为空
bool ring_buffer_is_empty(const RingBuffer *cb) {
    return (cb->count == 0);
}

// 检查缓冲区是否已满
bool ring_buffer_is_full(const RingBuffer *cb) {
    return (cb->count == RING_BUFFER_BUFFER_SIZE);
}

// 写入数据到缓冲区 (发送/生产)
bool ring_buffer_write(RingBuffer *cb, uint8_t data) {
    // 检查是否已满
    if (ring_buffer_is_full(cb)) {
        return false; // 缓冲区已满，写入失败
    }

    // 将数据写入 head 指向的位置
    cb->buffer[cb->head] = data;

    // 移动 head 指针，并使用模运算实现环形绕回
    cb->head = (cb->head + 1) % RING_BUFFER_BUFFER_SIZE;

    // 增加计数器
    cb->count++;

    return true; // 写入成功
}

// 从缓冲区读取数据 (接收/消费)
bool ring_buffer_read(RingBuffer *cb, uint8_t *data) {
    // 检查是否为空
    if (ring_buffer_is_empty(cb)) {
        return false; // 缓冲区为空，读取失败
    }

    // 从 tail 指向的位置读取数据
    *data = cb->buffer[cb->tail];

    // 移动 tail 指针，并使用模运算实现环形绕回
    cb->tail = (cb->tail + 1) % RING_BUFFER_BUFFER_SIZE;

    // 减少计数器
    cb->count--;

    return true; // 读取成功
}
