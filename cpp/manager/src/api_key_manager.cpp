// #include "manager/api_key_manager.hpp"
// #include "common/logger.hpp"
// #include <algorithm>

// namespace mlinference {
// namespace manager {

// ApiKeyManager::ApiKeyManager(const std::string& admin_api_key) {
//     if (!admin_api_key.empty()) {
//         admin_api_key_hash_ = hash_key(admin_api_key);
//         LOG_INFO("API Key Manager initialized with admin key");
//     } else {
//         LOG_WARN("API Key Manager initialized without admin key (insecure!)");
//     }
// }

// std::optional<std::string> ApiKeyManager::create_api_key(
//     const std::string& name,
//     const std::vector<common::Permission>& permissions,
//     std::optional<common::TimePoint> expires_at,
//     uint64_t rate_limit) {
    
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     // Generate new API key
//     std::string api_key = common::generate_api_key();
//     std::string key_hash = hash_key(api_key);
//     std::string key_id = common::generate_uuid();
    
//     ApiKeyInfo info;
//     info.key_id = key_id;
//     info.key_hash = key_hash;
//     info.name = name;
//     info.permissions = permissions;
//     info.enabled = true;
//     info.created_at = common::now();
//     info.expires_at = expires_at;
//     info.request_count = 0;
//     info.rate_limit_per_minute = rate_limit;
    
//     keys_[key_hash] = info;
    
//     LOG_INFO("API key created: id={}, name={}, permissions={}", 
//              key_id, name, permissions.size());
    
//     return api_key;  // Return plain key only once
// }

// bool ApiKeyManager::revoke_api_key(const std::string& key_id) {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     for (auto& [hash, info] : keys_) {
//         if (info.key_id == key_id) {
//             info.enabled = false;
//             LOG_INFO("API key revoked: id={}, name={}", key_id, info.name);
//             return true;
//         }
//     }
    
//     LOG_WARN("Attempted to revoke unknown key: {}", key_id);
//     return false;
// }

// bool ApiKeyManager::validate_api_key(
//     const std::string& api_key,
//     std::optional<common::Permission> required_permission) {
    
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     cleanup_expired_keys();
    
//     // Check admin key
//     if (hash_key(api_key) == admin_api_key_hash_) {
//         return true;  // Admin has all permissions
//     }
    
//     std::string key_hash = hash_key(api_key);
//     auto it = keys_.find(key_hash);
    
//     if (it == keys_.end()) {
//         LOG_DEBUG("Invalid API key");
//         return false;
//     }
    
//     const ApiKeyInfo& info = it->second;
    
//     if (!info.enabled) {
//         LOG_DEBUG("API key disabled: {}", info.name);
//         return false;
//     }
    
//     // Check expiration
//     if (info.expires_at && common::now() > *info.expires_at) {
//         LOG_DEBUG("API key expired: {}", info.name);
//         return false;
//     }
    
//     // Check permission
//     if (required_permission) {
//         auto perm_it = std::find(info.permissions.begin(), 
//                                 info.permissions.end(), 
//                                 *required_permission);
        
//         // Also check for ADMIN permission
//         auto admin_it = std::find(info.permissions.begin(),
//                                  info.permissions.end(),
//                                  common::Permission::ADMIN);
        
//         if (perm_it == info.permissions.end() && admin_it == info.permissions.end()) {
//             LOG_DEBUG("API key {} lacks required permission", info.name);
//             return false;
//         }
//     }
    
//     // Increment request count
//     const_cast<ApiKeyInfo&>(info).request_count++;
    
//     return true;
// }

// bool ApiKeyManager::is_admin_key(const std::string& api_key) const {
//     return hash_key(api_key) == admin_api_key_hash_;
// }

// std::optional<ApiKeyInfo> ApiKeyManager::get_key_info(const std::string& key_id) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     for (const auto& [hash, info] : keys_) {
//         if (info.key_id == key_id) {
//             return info;
//         }
//     }
    
//     return std::nullopt;
// }

// std::vector<ApiKeyInfo> ApiKeyManager::list_api_keys(bool include_disabled) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     std::vector<ApiKeyInfo> result;
//     result.reserve(keys_.size());
    
//     for (const auto& [hash, info] : keys_) {
//         if (info.enabled || include_disabled) {
//             result.push_back(info);
//         }
//     }
    
//     return result;
// }

// bool ApiKeyManager::check_rate_limit(const std::string& api_key) {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     reset_rate_limit_windows();
    
//     std::string key_hash = hash_key(api_key);
//     auto it = keys_.find(key_hash);
    
//     if (it == keys_.end()) {
//         return false;
//     }
    
//     const ApiKeyInfo& info = it->second;
    
//     // Check rate limit
//     auto& rate_info = rate_limits_[key_hash];
//     auto now = common::now();
    
//     // Reset window if needed
//     auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
//         now - rate_info.window_start);
    
//     if (elapsed.count() >= 60) {
//         rate_info.window_start = now;
//         rate_info.request_count = 0;
//     }
    
//     if (rate_info.request_count >= info.rate_limit_per_minute) {
//         LOG_DEBUG("Rate limit exceeded for key: {}", info.name);
//         return false;
//     }
    
//     rate_info.request_count++;
//     return true;
// }

// std::string ApiKeyManager::hash_key(const std::string& key) const {
//     return common::sha256_hash(key);
// }

// void ApiKeyManager::cleanup_expired_keys() {
//     auto now = common::now();
    
//     for (auto it = keys_.begin(); it != keys_.end();) {
//         if (it->second.expires_at && now > *it->second.expires_at) {
//             LOG_INFO("Removing expired key: {}", it->second.name);
//             it = keys_.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }

// void ApiKeyManager::reset_rate_limit_windows() {
//     auto now = common::now();
    
//     for (auto it = rate_limits_.begin(); it != rate_limits_.end();) {
//         auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
//             now - it->second.window_start);
        
//         if (elapsed.count() >= 120) {  // Remove after 2 minutes of inactivity
//             it = rate_limits_.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }

// } // namespace manager
// } // namespace mlinference