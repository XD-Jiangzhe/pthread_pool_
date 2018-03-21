#ifndef PTHREAD_POOL_H
#define PTHREAD_POOL_H

#include <assert.h>

#include <vector>
#include <deque>
#include <memory>
#include <algorithm>
#include <functional>
#include <mutex>
#include <thread>
#include <exception>
#include <condition_variable>

namespace pthread_pool
{
    using std::string;
    using std::vector;
    using std::deque;

    class threadPool{

        public:
             typedef  std::function<void()> Task;
             explicit threadPool(const string& nameArg = string("threadpool"));
            ~threadPool();

            void setMaxQueueSize(int maxsize){maxQueueSize = maxsize;}
            void setThreadInitialCallBack(const Task& cb){threadInitCallback_ = cb;}

            void start(int numThreads);
            void stop();
            void run(const Task& t);

            size_t queueSize();
            const string& name()const{return name_;}

        private:
            bool isFull()const;
            void runInThread();
            Task take();

        private:
            std::mutex mu_;
            std::condition_variable notEmpty_;
            std::condition_variable notFull_;
            string name_;
            Task threadInitCallback_;
            
            vector<std::thread> threads_;
            deque<Task> queue_;
            size_t maxQueueSize;
            bool running;
    };
}



#endif