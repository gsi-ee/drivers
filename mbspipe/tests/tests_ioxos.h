#ifndef __TESTS_IOXOS_H__
#define __TESTS_IOXOS_H__

/* JAM2021: try different length for this test. Since they are used both in producer and consumer, put into common include */
#define IOXOS_CHUNK_SIZE_INTS 0x400
//0xB
//0x10
// 0x61
//0x100
//0x40
//0x400
#define IOXOS_NUM_CHUNKS 0xA000
//0x3A2E8B
//0x280000
//0x6990F
//0x27FFF
//0xA0000
//0x1000

/* homw many times we repeat the test from beginning of pipe*/
#define IOXOS_PIPE_REPEATS 10000
//100


#endif
