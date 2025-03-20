#include "json.hpp"

namespace data_processing {

std::shared_ptr<json> json::create(){
    return std::shared_ptr<json>(new json());
}

void json::set(const std::string& key, const json_value& value) {
    data[key] = std::make_shared<json_value>(value);
}

std::string json::to_string() const {
    std::ostringstream oss;
    oss << "{";
    bool first = true;

    for (const auto& [key, value] : data) {
        if (!first) oss << ", ";
        first = false;

        oss << "\"" << key << "\": " << stringify(value);
    }

    oss << "}";
    return oss.str();
}

std::string json::stringify(const std::shared_ptr<json_value> value) {
    std::ostringstream oss;
    std::visit([&oss](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
            oss << "\"" << arg << "\"";
        else if constexpr (std::is_same_v<T, bool>)
            oss << (arg ? "true" : "false");
        else if constexpr (std::is_same_v<T, std::nullptr_t>)
            oss << "null";
        else if constexpr (std::is_same_v<T, std::vector<json>>) {
            oss << "[";
            bool first = true;
            for (const auto& item : arg) {
                if (!first) oss << ", ";
                first = false;
                oss << item.to_string();
            }
            oss << "]";
        }
        else if constexpr (std::is_same_v<T, std::shared_ptr<json>>) {
            oss << arg->to_string();
        }
        else{
            // handle int + double
            oss << arg;
        }
    }, *value);
    return oss.str();
}

} //namespace data_processing 