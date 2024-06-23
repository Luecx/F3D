#include "LoadingThread.h"

LoadingThread::LoadingThread() {
    logging::log(1, INFO, "Starting loading thread.");
    worker = std::thread(&LoadingThread::process_operations, this);
}

LoadingThread::~LoadingThread() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_thread = true;
    }
    queue_cv.notify_all();
    worker.join();
    logging::log(1, INFO, "Loading thread stopped.");
}

void LoadingThread::queue_operation(const Operation& operation) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        operations_queue.push(operation);
        logging::log(1, DEBUG, "Queued operation for resource: " + operation.data->get_path());
    }
    queue_cv.notify_one();
}

void LoadingThread::process_operations() {
    while (true) {
        Operation op;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [this] { return !operations_queue.empty() || stop_thread; });

            if (stop_thread && operations_queue.empty()) {
                logging::log(1, INFO, "Stopping processing operations.");
                break;
            }

            op = operations_queue.front();
            operations_queue.pop();
        }

        bool result = op.data->operator()(op.state, op.type);
        if (result) {
            logging::log(1, INFO, "Successfully processed operation for resource: " + op.data->get_path());
        } else {
            logging::log(1, WARNING, "Failed to process operation for resource: " + op.data->get_path());
        }
    }
}
