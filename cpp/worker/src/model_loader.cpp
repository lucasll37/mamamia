// #ifndef ML_INFERENCE_THREAD_POOL_HPP
// #define ML_INFERENCE_THREAD_POOL_HPP

// #include <vector>
// #include <queue>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <functional>
// #include <future>

// namespace mlinference {
// namespace worker {

// // Simple thread pool for handling concurrent inference requests
// class ThreadPool {
// public:
//     explicit ThreadPool(size_t num_threads);
//     ~ThreadPool();
    
//     // Submit a task to the pool
//     template<typename F, typename... Args>
//     auto submit(F&& f, Args&&... args) 
//         -> std::future<typename std::result_of<F(Args...)>::type>;
    
//     // Get number of threads
//     size_t size() const { return threads_.size(); }
    
//     // Get number of queued tasks
//     size_t queued() const;
    
// private:
//     std::vector<std::thread> threads_;
//     std::queue<std::function<void()>> tasks_;
    
//     std::mutex mutex_;
//     std::condition_variable condition_;
//     bool stop_;
    
//     void worker_thread();
// };

// // Template implementation
// template<typename F, typename... Args>
// auto ThreadPool::submit(F&& f, Args&&... args) 
//     -> std::future<typename std::result_of<F(Args...)>::type> {
    
//     using return_type = typename std::result_of<F(Args...)>::type;
    
//     auto task = std::make_shared<std::packaged_task<return_type()>>(
//         std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//     );
    
//     std::future<return_type> result = task->get_future();
    
//     {
//         std::unique_lock<std::mutex> lock(mutex_);
        
//         if (stop_) {
//             throw std::runtime_error("ThreadPool is stopped");
//         }
        
//         tasks_.emplace([task](){ (*task)(); });
//     }
    
//     condition_.notify_one();
//     return result;
// }

// } // namespace worker
// } // namespace mlinference

// #endif // ML_INFERENCE_THREAD_POOL_HPP