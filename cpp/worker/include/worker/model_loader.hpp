// #ifndef ML_INFERENCE_MODEL_LOADER_HPP
// #define ML_INFERENCE_MODEL_LOADER_HPP

// #include <string>
// #include <memory>
// #include <map>
// #include <mutex>
// #include "common/types.hpp"

// namespace mlinference {
// namespace worker {

// // Simplified Model Loader
// // In production, this would handle:
// // - Downloading models from Manager
// // - Caching models locally
// // - Managing model versions
// // - Lazy loading with timeouts

// class ModelLoader {
// public:
//     ModelLoader(const std::string& cache_directory,
//                uint64_t max_cache_size_mb);
//     ~ModelLoader() = default;
    
//     // Download model from Manager
//     bool download_model(const std::string& model_id,
//                        const std::string& version,
//                        const std::string& manager_address);
    
//     // Get local path to model
//     std::optional<std::string> get_model_path(const std::string& model_id,
//                                              const std::string& version) const;
    
//     // Check if model is cached
//     bool is_model_cached(const std::string& model_id,
//                         const std::string& version) const;
    
//     // Remove model from cache
//     bool remove_from_cache(const std::string& model_id,
//                           const std::string& version);
    
//     // Cleanup old models
//     void cleanup_cache();
    
// private:
//     std::string cache_directory_;
//     uint64_t max_cache_size_mb_;
    
//     mutable std::mutex mutex_;
//     std::map<std::string, common::TimePoint> model_access_times_;
// };

// } // namespace worker
// } // namespace mlinference

// #endif // ML_INFERENCE_MODEL_LOADER_HPP