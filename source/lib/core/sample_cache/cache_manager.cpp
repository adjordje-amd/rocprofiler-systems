#include "cache_manager.hpp"
#include "config.hpp"
#include "sample_cache/sample_type.hpp"

namespace rocprofsys
{
namespace sample_cache
{

cache_manager&
cache_manager::get_instance()
{
    static cache_manager instance;
    return instance;
}

cache_manager::cache_manager()
{
    // register callbacks
    parser.register_type_callback(entry_type::region,
                                  [](const storage_parsed_type_base&) {});
}

void
cache_manager::post_process()
{
    // TODO
}

void
cache_manager::shutdown()
{
    m_storage.shutdown();
}

}  // namespace sample_cache
}  // namespace rocprofsys
