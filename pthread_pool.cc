#include "pthread_pool.h"

using namespace pthread_pool;

threadPool::threadPool(const  string& nameArg)
    :name_(nameArg),maxQueueSize(0),running(false)
    {}

threadPool::~threadPool()
{
    if(running)
        stop();
}

void threadPool::start(int numThreads)
{
    assert(threads_.empty());

    running = true;
    threads_.reserve(numThreads);

    for(int i=0; i<numThreads; ++i)
    {
        threads_.push_back(std::thread(std::bind(&threadPool::runInThread, this)));
    }

    if(numThreads == 0 && threadInitCallback_)
        threadInitCallback_();
}


void threadPool::stop()
{
    {
        std::lock_guard<std::mutex> lg(mu_);
        running = false;
        notEmpty_.notify_all();
    }
    for(auto &i: threads_)
    {
        i.join();
    }
}

void threadPool::run(const Task& task)
{
    if(threads_.empty())
    {
        task();
    }
    else 
    {
        std::unique_lock<std::mutex> lg(mu_);
        while(isFull())
        {
            notFull_.wait(lg);
        }
        queue_.push_back(std::move(task));
        notEmpty_.notify_one();
    }
}

size_t threadPool::queueSize(){
    
    std::unique_lock<std::mutex> lg(mu_);
    return queue_.size();
}

bool threadPool::isFull()const{

    if(queue_.size() == maxQueueSize)
        return true;
    else 
        return false;
}

void threadPool::runInThread()
{
    try{
        while(running)
        {
            Task task(take());
            if(task)
                task();
        }
    }catch(const std::exception& ex)
    {
        fprintf(stderr, "%s", ex.what());
        abort();
    }
}

threadPool::Task threadPool::take()
{
    std::unique_lock<std::mutex> lg(mu_);
    while(queue_.empty() && running)
    {
        notEmpty_.wait(lg);
    }

    Task task;
    if(!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if(maxQueueSize> 0)
            notFull_.notify_one();
    }

    return task;
}











