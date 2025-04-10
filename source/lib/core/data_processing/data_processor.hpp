#pragma once 

#include <string>
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <functional>
#include <any>

#include "core/data_storage/database.hpp"
#include "core/data_storage/queries/table_insert_query.hpp"

namespace rocprofsys {
namespace data_processing {

struct data_processor {

    using insert_event_stmt = std::function<void(size_t, const char*, size_t, size_t, size_t, size_t, const char*, const char*, const char*)>;
    using insert_pmc_event_stms = std::function<void(size_t, const char*, size_t, size_t, double, const char*)>;
    using insert_sample_stmt = std::function<void(const char*, size_t, uint64_t, size_t, const char*)>;

    enum class category_id {
        smi_device_busy,
        smi_device_temperature,
        smi_device_power,
        smi_device_memory_usage,
    };
private: 
    struct pmc_identifier {
        size_t agent_id;
        std::string name;
    };


    struct pmc_identifier_hash {
        std::size_t operator()(const pmc_identifier& pmc) const noexcept {
            std::size_t h1 = std::hash<size_t>{}(pmc.agent_id);
            std::size_t h2 = std::hash<std::string>{}(pmc.name);
            return h1 ^ (h2 << 1);
        }
    };

    struct pmc_identifier_equal {
        bool operator()(const pmc_identifier& lhs, const pmc_identifier& rhs) const noexcept {
            return lhs.agent_id == rhs.agent_id && lhs.name == rhs.name;
        }
    };

public:
    enum class correlation_id {
        smi_unused
    };

    static data_processor& get_instance();

    size_t insert_string(const char* str);

    void insert_node_info(size_t node_id, size_t hash, const char* machine_id, const char* system_name, const char* hostname, 
                            const char* release, const char* version, const char* hardware_name, const char* domain_name);

    void insert_process_info(size_t node_id, size_t ppid, size_t pid, size_t init, size_t fini, size_t start, size_t end, 
                                const char* command, const char* environment = "{}", const char* extdata = "{}");

    void insert_agent(size_t agent_id, size_t node_id, size_t pid, const char* agent_type, size_t absolute_index, size_t logical_index, size_t type_index, uint64_t uuid, 
                        const char* name, const char* model_name, const char* vendor_name, const char* product_name, const char* user_name, const char* extdata = "{}");
    
    void insert_track(const char* track_name, size_t node_id, size_t process_id, size_t thread_id, const char* extdata = "{}");

    size_t insert_event(size_t category_id, size_t stack_id, size_t parent_stack_id, size_t correlation_id, const char* call_stack = "{}", 
                        const char* line_info = "{}", const char* extdata = "{}");
 
    void insert_pmc_event(size_t event_id, size_t agent_id, const char* pmc_descriptor, double value, const char* extdata = "{}");

    void insert_pmc_description(size_t node_id, size_t process_id, size_t agent_id, const char* target_arch, size_t event_code, size_t instance_id, const char* name, const char* symbol, 
                                const char* description, const char* long_description, const char* component, const char* units, const char* value_type, 
                                const char* block, const char* expression, uint32_t is_constant, uint32_t is_derived, const char* extdata = "{}");

    void insert_sample(const char* track, uint64_t timestamp, size_t event_id, const char* extdata = "{}");
    
private:
    data_processor();

    data_processor(data_processor&) = delete;
    
    data_processor& operator=(const data_processor&) = delete;
    
    void initialize_pmc_event_stmt();

    void initialize_event_stmt();

    void initialize_sample_stmt();

private:
    std::unordered_map<std::string, size_t> _track_name_map;
    std::unordered_map<pmc_identifier, size_t, pmc_identifier_hash, pmc_identifier_equal> _pmc_descriptor_map;
    std::unordered_map<size_t, size_t> _agent_id_map;

    insert_event_stmt _insert_event_statement;
    insert_pmc_event_stms _insert_pmc_event_statement;
    insert_sample_stmt _insert_sample_statement;
    size_t _pmc_id{1};
    size_t _event_id{1};
    size_t _pmc_event_id{1};
    size_t _sample_id{1};
    size_t _agent_id{1};
    size_t _string_id{1};
    const std::string _upid = "_R4nd0m1D";
};


} // namespace data_processing
} // namespace rocprofsys

