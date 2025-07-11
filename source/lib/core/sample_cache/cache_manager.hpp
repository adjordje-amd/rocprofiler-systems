#pragma once
#include "cache_storage.hpp"
#include "cache_storage_parser.hpp"
#include "metadata_storage.hpp"

namespace rocprofsys
{
namespace sample_cache
{

class cache_manager
{
public:
    static cache_manager& get_instance();
    cache_storage&        get_cache() { return m_storage; }
    metadata&             get_metadata() { return m_metadata; }

    void shutdown();

    void post_process();

private:
    cache_manager();
    cache_storage  m_storage;
    metadata       m_metadata;
    storage_parser parser;
};

}  // namespace sample_cache
}  // namespace rocprofsys
