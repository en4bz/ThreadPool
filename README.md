ThreadPool
==========

A simple C++11 Thread Pool implementation.

Note: The default constructor `ThreadPool<POLICY> name;` will create a ThreadPool with a number of worker threads 
equivelent to `std::thread::hardware_concurrency()` or in other words the name number of threads, 
including hyperthreading, of the underlying CPU.

Now with queueing policies!
 * `FIFO_POLICY` (default)
  * Creates a First In First Out ThreadPool, queueing submitted tasks using `std::queue`.
  * To create: `ThreadPool<> name(numberOfThreads);` or `ThreadPool<FIFO_POLICY> name(numberOfThreads);`
  * To enqeue: `enqueue(function, arguments...);`
 * `LIFO_POLICY`
  * Creates a Last In First Out ThreadPool, queueing submitted tasks using `std::stack`.
  * To create: `ThreadPool<LIFO_POLICY> name(numberOfThreads);`
  * To enqeue: `enqueue(function, arguments...);`
 * `PRIORITY_POLICY`
  * Creates a prioritized ThreadPool, queueing submitted tasks using `std::priority_queue`. 
  * To create: `ThreadPool<PRIORITY_POLICY> name(numberOfThreads);`
  * To enqeue: `enqueue(priority, function, arguments...);`
  * Tasks with higher (greater integer value) priorites will be dequeued before ones with lower priority.  
    For example:  
<pre>
<code>
        ThreadPool&lt;PRIORITY_POLICY> tp;
        tp.enqueue(10, func1, args...);
        tp.enqueue(2, func2, args...);
</code>
</pre>
    
      will cause `func1` to be dequeued before `func2` assuming that both tasks are submitted at the same time. 
