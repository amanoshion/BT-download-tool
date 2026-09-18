#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "protocol.h"

#define MAX_EVENTS 1024
#define THREAD_NUM 24
#define BUFFER_SIZE 4096
#define LISTEN_BACKLOG 128

/* ========== thread ============*/
typedef struct {
        int fd;         // client sockfd 
        uint32_t events;        // event of epoll return
} task_t;

typedef struct {
        Task *queue;            /* task queue */
        int capacity;
        int head;
        int tail;
        int count;

        pthread_mutex_t mutex;  // mutex
        pthread_cond_t not_empty;
        pthread_cond_t not_full;

        pthread_t *threads;     /* working task's thread */
        int thread_num;
        int stop;
} thread_pool_t;

int set_nonblocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return -1;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void epoll_mod(int epfd, int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events | EPOLLET | EPOLLONESHOT;
        ev.data.fd = fd;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                perror("epoll_ctl MOD");
        }
}

void pool_add_task(thread_pool_t *pool, int fd, uint32_t events) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->count == pool->capacity && !pool->stop) {
                pthread_cond_wait(&pool->not_full, &pool->mutex);
        }
        if (pool->stop) {
                pthread_mutex_unlock(&pool->mutex);
                return;
        }
        pool->queue[pool->tail].fd = fd;
        pool->queue[pool->tail].events = events;
        pool->tail = (pool->tail + 1) % pool->capacity;
        pool->count++;

        pthread_cond_signal(&pool->not_empty);
        pthread_mutex_unlock(&pool->mutex);
}

void *worker_thread(void *arg) {
        thread_pool_t *pool = (thread_pool_t *)arg;
        char buffer[BUFFER_SIZE];

        while(1) {
                pthread_mutex_lock(&pool->mutex);

                while(pool->count == 0 && !pool->stop) {
                        pthread_cond_wait(&pool->not_empty, &pool->mutex);
                }
                if (pool->stop && pool->count == 0) {
                        pthread_mutex_unlock(&pool->mutex);
                        break;
                }

                task_t task = pool->queue[pool->head];
                pool->head = (pool->head + 1) % pool->capacity;
                pool->count--;
                pthread_cond_signal(&pool->not_full);
                pthread_mutex_unlock(&pool->mutex);

                int fd = task.fd;

                if (task.events & EPOLLIN) {
                        while(1) {
                                ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
                                if (n > 0) {
                                        buffer[n] = '\0';
                                        printf("[thread %lu] recieve [client %d]: %s\n",
                                                pthread_self(), fd, buffer
                                        );
                                } else if (n == 0) {
                                        printf("[thread %lu] [client %d] close\n",
                                                pthread_self(), fd
                                        );
                                } else {
                                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                                break;
                                        } else {
                                                perror("read");
                                                close(fd);
                                                fd = -1;
                                                break;
                                        }
                                }
                        }
                }
                if (fd != -1) {
                        epoll_mod(g_epfd, fd, EPOLLIN);
                }
        }
        return NULL;
}

int pool_init(thread_pool_t *pool, int thread_num, int queue_size) {
        pool->capacity          = queue_size;
        pool->head              = pool->tail = pool->cond = 0;
        pool->stop              = 0;
        pool->thread_num        = thread_num;

        pool->queue             = calloc(queue_size, sizeof(task_t));
        pool->threads           = calloc(thread_num, sizeof(pthread_t));
        if (!pool->queue || !pool->threads) return -1;

        pool->queue             = calloc(queue_size, sizeof(task_t));
        pool->threads           = calloc(thread_num, sizeof(pthread_t));
        if (!pool->queue || !pool->threads) return -1;

        pthread_mutex_init(&pool->mutex, NULL);
        pthread_cond_init(&pool->not_empty, NULL);
        pthread_cond_init(&pool->not_full, NULL);

        for (int i = 0; i < thread_num; i++) {
                if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
                        return -1;
                }
        }
        return 0;
}

void pool_destroy(thread_pool_t *pool) {
        pthread_mutex_lock(&pool->mutex);
        pool->stop = 1;
        pthread_cond_broadcast(&pool->not_empty);
        pthread_cond_broadcast(&pool->not_full);
        pthread_mutex_unlock(&pool->mutex);

        for (int i = 0; i < pool->thread_num; i++) {
                pthread_join(pool->threads[i], NULL);
        }
        free(pool->queue);
        free(pool->threads);
        pthread_mutex_destroy(&pool->mutex);
        pthread_cond_destroy(&pool->not_empty);
        pthread_cond_destroy(&pool->not_full);
}
#endif

