// #ifndef ML_INFERENCE_COMMON_TYPES_HPP
// #define ML_INFERENCE_COMMON_TYPES_HPP

// #include <string>
// #include <vector>
// #include <map>
// #include <memory>
// #include <chrono>
// #include <optional>

// namespace mlinference {
// namespace common {

// // Type aliases for clarity
// using TimePoint = std::chrono::system_clock::time_point;
// using Duration = std::chrono::milliseconds;
// using ByteVector = std::vector<uint8_t>;

// // Semantic versioning
// struct Version {
//     uint32_t major{0};
//     uint32_t minor{0};
//     uint32_t patch{0};
    
//     Version() = default;
//     Version(uint32_t maj, uint32_t min, uint32_t pat)
//         : major(maj), minor(min), patch(pat) {}
    
//     std::string to_string() const;
//     static Version from_string(const std::string& version_str);
    
//     bool operator==(const Version& other) const;
//     bool operator!=(const Version& other) const;
//     bool operator<(const Version& other) const;
//     bool operator>(const Version& other) const;
//     bool operator<=(const Version& other) const;
//     bool operator>=(const Version& other) const;
// };

// // Model stage enum
// enum class ModelStage {
//     DEV,
//     STAGING,
//     PRODUCTION
// };

// std::string model_stage_to_string(ModelStage stage);
// ModelStage model_stage_from_string(const std::string& stage_str);

// // Tensor data types
// enum class TensorDataType {
//     FLOAT32,
//     FLOAT64,
//     INT32,
//     INT64,
//     UINT8,
//     BOOL,
//     STRING
// };

// std::string tensor_dtype_to_string(TensorDataType dtype);
// TensorDataType tensor_dtype_from_string(const std::string& dtype_str);
// size_t tensor_dtype_size(TensorDataType dtype);

// // Tensor shape
// using TensorShape = std::vector<int64_t>;

// // Calculate total elements in tensor
// int64_t tensor_total_elements(const TensorShape& shape);

// // Worker status enum
// enum class WorkerStatus {
//     UNKNOWN,
//     IDLE,
//     BUSY,
//     OVERLOADED,
//     OFFLINE
// };

// std::string worker_status_to_string(WorkerStatus status);
// WorkerStatus worker_status_from_string(const std::string& status_str);

// // API Key permissions
// enum class Permission {
//     INFERENCE,
//     MODEL_UPLOAD,
//     MODEL_DELETE,
//     WORKER_REGISTER,
//     ADMIN
// };

// std::string permission_to_string(Permission perm);
// Permission permission_from_string(const std::string& perm_str);

// // Result type for operations that can fail
// template<typename T, typename E = std::string>
// class Result {
// public:
//     Result(T value) : value_(std::move(value)), is_ok_(true) {}
//     Result(E error) : error_(std::move(error)), is_ok_(false) {}
    
//     bool is_ok() const { return is_ok_; }
//     bool is_err() const { return !is_ok_; }
    
//     const T& value() const {
//         if (!is_ok_) {
//             throw std::runtime_error("Attempted to get value from error Result");
//         }
//         return value_;
//     }
    
//     T& value() {
//         if (!is_ok_) {
//             throw std::runtime_error("Attempted to get value from error Result");
//         }
//         return value_;
//     }
    
//     const E& error() const {
//         if (is_ok_) {
//             throw std::runtime_error("Attempted to get error from ok Result");
//         }
//         return error_;
//     }
    
//     // Move semantics
//     T value_or(T default_value) {
//         return is_ok_ ? std::move(value_) : std::move(default_value);
//     }
    
// private:
//     T value_;
//     E error_;
//     bool is_ok_;
// };

// // Simple UUID generator
// std::string generate_uuid();

// // Timestamp utilities
// std::string timestamp_to_string(const TimePoint& tp);
// TimePoint timestamp_from_string(const std::string& timestamp_str);
// TimePoint now();

// // Hash utilities
// std::string sha256_hash(const std::string& data);
// std::string generate_api_key();

// // File utilities
// bool file_exists(const std::string& path);
// size_t file_size(const std::string& path);
// std::optional<ByteVector> read_file(const std::string& path);
// bool write_file(const std::string& path, const ByteVector& data);

// } // namespace common
// } // namespace mlinference

// #endif // ML_INFERENCE_COMMON_TYPES_HPP
