#ifndef __USER_TIMER_H__
#define __USER_TIMER_H__

typedef void (*ptimer_cb_t)(void *arg);

uintptr_t ptimer_start_once(uint32_t ms, ptimer_cb_t cb, void *arg);

int ptimer_stop(uintptr_t h);

#endif /* __USER_TIMER_H__ */
