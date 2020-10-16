/*
 * queue.hpp
 *
 *  Created on: 14.10.2020
 *      Author: Andreas
 */

#ifndef SRC_OS_WIN32_QUEUE_HPP_
#define SRC_OS_WIN32_QUEUE_HPP_


#include <windows.h>

// taken from https://stackoverflow.com/questions/3054701/is-there-a-set-of-win32-api-functions-to-manage-synchronized-queues

template<class T, unsigned max = 256>
class cQueue {
    HANDLE space_avail; // at least one slot empty
    HANDLE data_avail;  // at least one slot full
    CRITICAL_SECTION mutex; // protect buffer, in_pos, out_pos

    T buffer[max];
    int in_pos, out_pos;
public:
    cQueue() : in_pos(0), out_pos(0) {
        space_avail = CreateSemaphore(NULL, max, max, NULL);
        data_avail = CreateSemaphore(NULL, 0, max, NULL);
        InitializeCriticalSection(&mutex);
    }

    void push(T data) {
        WaitForSingleObject(space_avail, INFINITE);
        EnterCriticalSection(&mutex);
        buffer[in_pos] = data;
        in_pos = (in_pos + 1) % max;
        LeaveCriticalSection(&mutex);
        ReleaseSemaphore(data_avail, 1, NULL);
    }

    T pop() {
        WaitForSingleObject(data_avail,INFINITE);
        EnterCriticalSection(&mutex);
        T retval = buffer[out_pos];
        out_pos = (out_pos + 1) % max;
        LeaveCriticalSection(&mutex);
        ReleaseSemaphore(space_avail, 1, NULL);
        return retval;
    }

    ~cQueue() {
        DeleteCriticalSection(&mutex);
        CloseHandle(data_avail);
        CloseHandle(space_avail);
    }
};



#endif /* SRC_OS_WIN32_QUEUE_HPP_ */
