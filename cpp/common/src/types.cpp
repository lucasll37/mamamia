// #include "common/types.hpp"
// #include <sstream>
// #include <iomanip>
// #include <random>
// #include <fstream>
// #include <ctime>
// #include <openssl/sha.h>

// namespace mlinference {
// namespace common {

// // ============================================================================
// // Version Implementation
// // ============================================================================

// std::string Version::to_string() const {
//     std::ostringstream oss;
//     oss << major << "." << minor << "." << patch;
//     return oss.str();
// }

// Version Version::from_string(const std::string& version_str) {
//     Version v;
//     char dot1, dot2;
//     std::istringstream iss(version_str);
    
//     if (!(iss >> v.major >> dot1 >> v.minor >> dot2 >> v.patch) ||
//         dot1 != '.' || dot2 != '.') {
//         // Return 0.0.0 on parse error
//         return Version{0, 0, 0};
//     }
    
//     return v;
// }

// bool Version::operator==(const Version& other) const {
//     return major == other.major && minor == other.minor && patch == other.patch;
// }

// bool Version::operator!=(const Version& other) const {
//     return !(*this == other);
// }

// bool Version::operator<(const Version& other) const {
//     if (major != other.major) return major < other.major;
//     if (minor != other.minor) return minor < other.minor;
//     return patch < other.patch;
// }

// bool Version::operator>(const Version& other) const {
//     return other < *this;
// }

// bool Version::operator<=(const Version& other) const {
//     return !(other < *this);
// }

// bool Version::operator>=(const Version& other) const {
//     return !(*this < other);
// }

// // ============================================================================
// // ModelStage Conversion
// // ============================================================================

// std::string model_stage_to_string(ModelStage stage) {
//     switch (stage) {
//         case ModelStage::DEV: return "dev";
//         case ModelStage::STAGING: return "staging";
//         case ModelStage::PRODUCTION: return "prod";
//         default: return "unknown";
//     }
// }

// ModelStage model_stage_from_string(const std::string& stage_str) {
//     if (stage_str == "dev") return ModelStage::DEV;
//     if (stage_str == "staging") return ModelStage::STAGING;
//     if (stage_str == "prod" || stage_str == "production") return ModelStage::PRODUCTION;
//     return ModelStage::DEV;
// }

// // ============================================================================
// // TensorDataType Conversion
// // ============================================================================

// std::string tensor_dtype_to_string(TensorDataType dtype) {
//     switch (dtype) {
//         case TensorDataType::FLOAT32: return "float32";
//         case TensorDataType::FLOAT64: return "float64";
//         case TensorDataType::INT32: return "int32";
//         case TensorDataType::INT64: return "int64";
//         case TensorDataType::UINT8: return "uint8";
//         case TensorDataType::BOOL: return "bool";
//         case TensorDataType::STRING: return "string";
//         default: return "unknown";
//     }
// }

// TensorDataType tensor_dtype_from_string(const std::string& dtype_str) {
//     if (dtype_str == "float32") return TensorDataType::FLOAT32;
//     if (dtype_str == "float64") return TensorDataType::FLOAT64;
//     if (dtype_str == "int32") return TensorDataType::INT32;
//     if (dtype_str == "int64") return TensorDataType::INT64;
//     if (dtype_str == "uint8") return TensorDataType::UINT8;
//     if (dtype_str == "bool") return TensorDataType::BOOL;
//     if (dtype_str == "string") return TensorDataType::STRING;
//     return TensorDataType::FLOAT32;
// }

// size_t tensor_dtype_size(TensorDataType dtype) {
//     switch (dtype) {
//         case TensorDataType::FLOAT32: return 4;
//         case TensorDataType::FLOAT64: return 8;
//         case TensorDataType::INT32: return 4;
//         case TensorDataType::INT64: return 8;
//         case TensorDataType::UINT8: return 1;
//         case TensorDataType::BOOL: return 1;
//         case TensorDataType::STRING: return 0; // Variable size
//         default: return 0;
//     }
// }

// // ============================================================================
// // Tensor Shape Utilities
// // ============================================================================

// int64_t tensor_total_elements(const TensorShape& shape) {
//     if (shape.empty()) return 0;
    
//     int64_t total = 1;
//     for (int64_t dim : shape) {
//         total *= dim;
//     }
//     return total;
// }

// // ============================================================================
// // WorkerStatus Conversion
// // ============================================================================

// std::string worker_status_to_string(WorkerStatus status) {
//     switch (status) {
//         case WorkerStatus::UNKNOWN: return "unknown";
//         case WorkerStatus::IDLE: return "idle";
//         case WorkerStatus::BUSY: return "busy";
//         case WorkerStatus::OVERLOADED: return "overloaded";
//         case WorkerStatus::OFFLINE: return "offline";
//         default: return "unknown";
//     }
// }

// WorkerStatus worker_status_from_string(const std::string& status_str) {
//     if (status_str == "idle") return WorkerStatus::IDLE;
//     if (status_str == "busy") return WorkerStatus::BUSY;
//     if (status_str == "overloaded") return WorkerStatus::OVERLOADED;
//     if (status_str == "offline") return WorkerStatus::OFFLINE;
//     return WorkerStatus::UNKNOWN;
// }

// // ============================================================================
// // Permission Conversion
// // ============================================================================

// std::string permission_to_string(Permission perm) {
//     switch (perm) {
//         case Permission::INFERENCE: return "inference";
//         case Permission::MODEL_UPLOAD: return "model_upload";
//         case Permission::MODEL_DELETE: return "model_delete";
//         case Permission::WORKER_REGISTER: return "worker_register";
//         case Permission::ADMIN: return "admin";
//         default: return "unknown";
//     }
// }

// Permission permission_from_string(const std::string& perm_str) {
//     if (perm_str == "inference") return Permission::INFERENCE;
//     if (perm_str == "model_upload") return Permission::MODEL_UPLOAD;
//     if (perm_str == "model_delete") return Permission::MODEL_DELETE;
//     if (perm_str == "worker_register") return Permission::WORKER_REGISTER;
//     if (perm_str == "admin") return Permission::ADMIN;
//     return Permission::INFERENCE;
// }

// // ============================================================================
// // UUID Generation
// // ============================================================================

// std::string generate_uuid() {
//     static std::random_device rd;
//     static std::mt19937_64 gen(rd());
//     static std::uniform_int_distribution<uint64_t> dis;
    
//     std::ostringstream oss;
//     oss << std::hex << std::setfill('0');
    
//     // Generate UUID v4 format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
//     uint64_t part1 = dis(gen);
//     uint64_t part2 = dis(gen);
    
//     oss << std::setw(8) << (part1 >> 32)
//         << "-" << std::setw(4) << ((part1 >> 16) & 0xFFFF)
//         << "-4" << std::setw(3) << ((part1 >> 4) & 0xFFF)
//         << "-" << std::setw(4) << ((part2 >> 48) & 0x3FFF | 0x8000)
//         << "-" << std::setw(12) << (part2 & 0xFFFFFFFFFFFF);
    
//     return oss.str();
// }

// // ============================================================================
// // Timestamp Utilities
// // ============================================================================

// std::string timestamp_to_string(const TimePoint& tp) {
//     auto time = std::chrono::system_clock::to_time_t(tp);
//     auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
//         tp.time_since_epoch()) % 1000;
    
//     std::ostringstream oss;
//     oss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S");
//     oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    
//     return oss.str();
// }

// TimePoint timestamp_from_string(const std::string& timestamp_str) {
//     std::tm tm = {};
//     std::istringstream iss(timestamp_str);
    
//     iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
//     auto time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
//     // Try to parse milliseconds if present
//     if (iss.peek() == '.') {
//         int ms;
//         iss.ignore(1) >> ms;
//         time += std::chrono::milliseconds(ms);
//     }
    
//     return time;
// }

// TimePoint now() {
//     return std::chrono::system_clock::now();
// }

// // ============================================================================
// // Hash Utilities
// // ============================================================================

// std::string sha256_hash(const std::string& data) {
//     unsigned char hash[SHA256_DIGEST_LENGTH];
//     SHA256_CTX sha256;
//     SHA256_Init(&sha256);
//     SHA256_Update(&sha256, data.c_str(), data.length());
//     SHA256_Final(hash, &sha256);
    
//     std::ostringstream oss;
//     for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
//         oss << std::hex << std::setw(2) << std::setfill('0') 
//             << static_cast<int>(hash[i]);
//     }
    
//     return oss.str();
// }

// std::string generate_api_key() {
//     static const char charset[] = 
//         "0123456789"
//         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//         "abcdefghijklmnopqrstuvwxyz";
//     static const size_t max_index = sizeof(charset) - 1;
    
//     static std::random_device rd;
//     static std::mt19937 gen(rd());
//     static std::uniform_int_distribution<> dis(0, max_index - 1);
    
//     const size_t length = 32;
//     std::string api_key;
//     api_key.reserve(length + 3); // +3 for prefix
    
//     api_key = "sk-";
//     for (size_t i = 0; i < length; ++i) {
//         api_key += charset[dis(gen)];
//     }
    
//     return api_key;
// }

// // ============================================================================
// // File Utilities
// // ============================================================================

// bool file_exists(const std::string& path) {
//     std::ifstream file(path);
//     return file.good();
// }

// size_t file_size(const std::string& path) {
//     std::ifstream file(path, std::ios::binary | std::ios::ate);
//     if (!file.good()) return 0;
//     return static_cast<size_t>(file.tellg());
// }

// std::optional<ByteVector> read_file(const std::string& path) {
//     std::ifstream file(path, std::ios::binary);
//     if (!file.good()) {
//         return std::nullopt;
//     }
    
//     file.seekg(0, std::ios::end);
//     size_t size = file.tellg();
//     file.seekg(0, std::ios::beg);
    
//     ByteVector data(size);
//     file.read(reinterpret_cast<char*>(data.data()), size);
    
//     if (!file.good()) {
//         return std::nullopt;
//     }
    
//     return data;
// }

// bool write_file(const std::string& path, const ByteVector& data) {
//     std::ofstream file(path, std::ios::binary);
//     if (!file.good()) {
//         return false;
//     }
    
//     file.write(reinterpret_cast<const char*>(data.data()), data.size());
//     return file.good();
// }

// } // namespace common
// } // namespace mlinference