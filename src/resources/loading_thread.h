#ifndef LOADINGTHREAD_H
#define LOADINGTHREAD_H

#include "resource_data.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "../logging/logging.h"

struct Operation {
    std::shared_ptr<ResourceData> data;
    Type type;
    State state;
};

class LoadingThread {
    private:
    std::queue<Operation> operations_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::thread worker;
    bool stop_thread = false;

    void process_operations();

    public:
    LoadingThread();
    ~LoadingThread();

    void queue_operation(const Operation& operation);
};

#endif // LOADINGTHREAD_H
