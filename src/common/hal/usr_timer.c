#include "common.h"
#include "usr_timer.h"
#include <time.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
	timer_t        tid;
	ptimer_cb_t    cb;
	void          *arg;
} timer_ctx_t;

static void proxy_cb(union sigval sv)
{
	timer_ctx_t *ctx = (timer_ctx_t *)sv.sival_ptr;

	if (ctx->cb)
		ctx->cb(ctx->arg);

	timer_delete(ctx->tid);
	free(ctx);
}

uintptr_t ptimer_start_once(uint32_t ms, ptimer_cb_t cb, void *arg)
{
	timer_ctx_t *ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return 0;

	ctx->cb  = cb;
	ctx->arg = arg;

	struct sigevent sev = {
		.sigev_notify            = SIGEV_THREAD,
		.sigev_notify_function   = proxy_cb,
		.sigev_value.sival_ptr   = ctx
	};

	if (timer_create(CLOCK_MONOTONIC, &sev, &ctx->tid) == -1) {
		free(ctx);
		return 0;
	}

	struct itimerspec its = {
		.it_value    = {
			.tv_sec  =  ms / 1000,
			.tv_nsec = (ms % 1000) * 1000000L
		},
		.it_interval = { 0 }
	};

	if (timer_settime(ctx->tid, 0, &its, NULL) == -1) {
		timer_delete(ctx->tid);
		free(ctx);
		return 0;
	}

	return (uintptr_t)ctx;
}

int ptimer_stop(uintptr_t h)
{
	if (!h)
		return -1;
	timer_ctx_t *ctx = (timer_ctx_t *)h;

	timer_delete(ctx->tid);
	free(ctx);
	return 0;
}
