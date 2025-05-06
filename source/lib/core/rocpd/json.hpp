#pragma once

#include <variant>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

namespace rocpd {

class json {
public:
    static std::shared_ptr<json> create();

    using json_value = std::variant<std::string, int, double, long long, bool, std::vector<json>, std::nullptr_t, std::shared_ptr<json>>;

    void set(const std::string& key, const json_value& value);

    std::string to_string() const;
private: 
    json() = default;

private:
    static std::string stringify(const std::shared_ptr<json_value> value);

private:
    std::unordered_map<std::string, std::shared_ptr<json_value>> data;

};

} // namespace rocpd 
    