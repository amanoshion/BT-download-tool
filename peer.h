#ifndef PEER_H
#define PEER_H
#define NOTOK -1
#define OK 0
typedef struct Puzzle{
        long peer_block_startsize;
        int peer_block_size;
        long peer_block_endsize;
        int puzzel_id;
        _Bool isok;     // 0 : ok ; -1 : not ok
} Puzzle;

typedef enum Download_Status {
        idle,
        prepare_download,
        downloading,
        downloaded
} Download_Status;

#endif