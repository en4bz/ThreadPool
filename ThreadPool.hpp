/*
 *Copyright (c) 2012 Jakob Progsch
 *
 *This software is provided 'as-is', without any express or implied
 *warranty. In no event will the authors be held liable for any damages
 *arising from the use of this software.
 *
 *Permission is granted to anyone to use this software for any purpose,
 *including commercial applications, and to alter it and redistribute it
 *freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.

 *  3. This notice may not be removed or altered from any source
 *  distribution.
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <stdexcept>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t = std::thread::hardware_concurrency() );//Default thread count is number of CPU threads
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;

	std::atomic<bool> isActive;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) : isActive(true)
{
    for(size_t i = 0 ; i < threads; i++)
        workers.push_back(std::thread(
            [this]
            {
                while(true)
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    while( this->isActive.load() && this->tasks.empty())
                        this->condition.wait(lock);
                    if( ! this->isActive && this->tasks.empty())
                        return;
                    std::function<void()> task(this->tasks.front());
                    this->tasks.pop();
                    lock.unlock();
                    task();
                }
            }
        ));
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>
{
    typedef decltype(std::forward<F>(f)(std::forward<Args>(args)...)) return_type;

    // don't allow enqueueing after stopping the pool
    if( ! isActive.load() )
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	this->isActive = false;
    condition.notify_all();
    for(std::thread& t : workers)
        t.join();
}

#endif
