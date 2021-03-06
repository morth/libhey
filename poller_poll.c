/*
 * Copyright (c) 2013 Per Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>

#include <poll.h>

#include "poller.h"
#include "timefuncs.h"

int
hey_poller_init(struct hey_poller *poller)
{
	return 0;
}

int
hey_poller_cleanup(struct hey_poller *poller)
{
	return 0;
}

void
hey_poller_reset_timeout(struct hey_poller *poller, const struct timespec *absto)
{
	poller->absto = *absto;
}

int
hey_poller_poll(struct hey_poller *poller, int *fds, int nfds, bool *ready)
{
	struct pollfd events[POLLER_MAX_FDS];
	int i;
	int r;
	struct timespec to;

	for (i = 0 ; i < nfds ; i++)
	{
		events[i].fd = fds[i];
		events[i].events = POLLOUT;
		events[i].revents = 0;
	}

	do {
		if (ts_now(&to))
			return -1;
		if (ts_cmp(&to, &poller->absto) > 0)
			to.tv_sec = to.tv_nsec = 0;
		else
			ts_sub_ts(&to, &poller->absto, &to);
		r = poll(events, nfds, to.tv_sec * 1000 + to.tv_nsec / 1000000);
	} while (r == -1 && errno == EINTR);

	if (r < 0)
		return POLLER_FATAL;
	if (r == 0)
		return POLLER_TIMEOUT;

	for (i = 0 ; i < nfds ; i++)
	{
		if (events[i].revents)
			break;
	}
	*ready = events[i].revents & POLLHUP ? false : true;
	return i;
}

