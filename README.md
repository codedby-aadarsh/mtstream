# mtstream

A multithreaded producer-consumer pipeline in C, simulating a real-time frame streaming architecture using POSIX threads.

## What it does

Two threads run concurrently inside the same process:

- **Producer thread** — generates fake JPEG frames at ~10fps and writes them to a shared buffer
- **Consumer thread** — reads frames from the buffer and prints them as they arrive

Synchronisation is handled with a `pthread_mutex` and `pthread_cond` — the consumer sleeps until the producer signals that a new frame is ready. Clean shutdown on `SIGINT` (Ctrl+C) is handled via a signal handler that wakes all sleeping threads before exiting.

## Why I built this

My internship involves building an MJPEG streaming pipeline on an ARM camera SoC — camera firmware writes JPEG frames to `/tmp`, and a CGI process serves them over HTTP. That pipeline is sequential: one write, one read, repeat.

mtstream models what a better architecture looks like — a dedicated capture thread and a dedicated streaming thread running in parallel, sharing frames through a mutex-protected buffer. This is how a production MJPEG server would be structured internally.

## Architecture

```
┌─────────────────┐         shared buffer          ┌─────────────────┐
│ Producer thread │  ──── mutex + cond_signal ──▶  │ Consumer thread │
│ (frame capture) │                                │ (frame serving) │
└─────────────────┘                                └─────────────────┘
        │                                                   │
   locks mutex                                        waits on cond
   writes frame                                       reads frame
   signals consumer                                   resets ready flag
   unlocks + sleeps                                   unlocks
```

## Build

```bash
make
```

## Clean 
```bash
make clean
```

## Run

```bash
./mtstream
```

Press `Ctrl+C` to stop. Both threads exit cleanly.

## Expected output

```
Consumer: frame 1 — JPEG_DATA_1
Consumer: frame 2 — JPEG_DATA_2
Consumer: frame 3 — JPEG_DATA_3
...
^CFrame 17: JPEG DATA 17
program safely exited!!
```

## What I learned building this

- `pthread_mutex_lock` / `pthread_mutex_unlock` for mutual exclusion — protecting shared buffer state from concurrent access
- `pthread_cond_wait` atomically releases the mutex and puts the thread to sleep — it reacquires the mutex before returning when signalled
- `pthread_cond_signal` vs `pthread_cond_broadcast` — signal wakes one waiter, broadcast wakes all — broadcast is needed in the signal handler to wake a sleeping consumer on shutdown
- Why the inner wait loop must check `&& running` — the consumer can wake up from broadcast with `ready == 0` (no new frame, just a shutdown signal) and must not process stale data
- Why `usleep` goes outside the mutex lock — sleeping while holding a lock blocks the consumer for the entire sleep duration
- `volatile int running` — prevents the compiler from caching the flag in a register and missing the signal handler's update

## Concepts demonstrated

- POSIX threads (`pthreads`) — `pthread_create`, `pthread_join`
- Mutex synchronisation — `pthread_mutex_t`
- Condition variables — `pthread_cond_t`, `pthread_cond_wait`, `pthread_cond_signal`, `pthread_cond_broadcast`
- Signal handling — `SIGINT`, `signal()`
- Producer-consumer pattern in C
- Clean thread shutdown without `pthread_cancel`

## Related projects

- [procmon](https://github.com/codedby-aadarsh/procmon) — Linux process monitor using `/proc` filesystem
- [shmstream](https://github.com/codedby-aadarsh/shmstream) — same producer-consumer pattern but across two separate processes using POSIX shared memory instead of threads