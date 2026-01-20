// #ifndef ML_INFERENCE_API_KEY_MANAGER_HPP
// #define ML_INFERENCE_API_KEY_MANAGER_HPP

// #include <string>
// #include <map>
// #include <mutex>
// #include <vector>
// #include <optional>
// #include "common/types.hpp"
// #include "common.pb.h"

// namespace mlinference {
// namespace manager {

// struct ApiKeyInfo {
//     std::string key_id;
//     std::string key_hash;  // SHA256 hash of the key
//     std::string name;
//     std::vector<common::Permission> permissions;
//     bool enabled;
//     common::TimePoint created_at;
//     std::optional<common::TimePoint> expires_at;
//     uint64_t request_count;
//     uint64_t rate_limit_per_minute;
// };

// class ApiKeyManager {
// public:
//     explicit ApiKeyManager(const std::string& admin_api_key);
//     ~ApiKeyManager() = default;
    
//     // Key management
//     std::optional<std::string> create_api_key(
//         const std::string& name,
//         const std::vector<common::Permission>& permissions,
//         std::optional<common::TimePoint> expires_at = std::nullopt,
//         uint64_t rate_limit = 1000);
    
//     bool revoke_api_key(const std::string& key_id);
    
//     // Validation
//     bool validate_api_key(const std::string& api_key,
//                          std::optional<common::Permission> required_permission = std::nullopt);
    
//     bool is_admin_key(const std::string& api_key) const;
    
//     // Queries
//     std::optional<ApiKeyInfo> get_key_info(const std::string& key_id) const;
    
//     std::vector<ApiKeyInfo> list_api_keys(bool include_disabled = false) const;
    
//     // Rate limiting
//     bool check_rate_limit(const std::string& api_key);
    
// private:
//     mutable std::mutex mutex_;
//     std::map<std::string, ApiKeyInfo> keys_;  // key_hash -> info
//     std::string admin_api_key_hash_;
    
//     // Rate limiting tracking
//     struct RateLimitInfo {
//         common::TimePoint window_start;
//         uint64_t request_count;
//     };
//     std::map<std::string, RateLimitInfo> rate_limits_;
    
//     // Helper methods
//     std::string hash_key(const std::string& key) const;
//     void cleanup_expired_keys();
//     void reset_rate_limit_windows();
// };

// } // namespace manager
// } // namespace mlinference

// #endif // ML_INFERENCE_API_KEY_MANAGER_HPP