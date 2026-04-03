#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace benchmark {

template<typename T>
class Registry {
public:
    struct Entry {
        std::string name;
        std::function<std::unique_ptr<T>()> factory;
    };

    static Registry& instance() {
        static Registry reg;
        return reg;
    }

    void add(const std::string& name, std::function<std::unique_ptr<T>()> factory) {
        entries_.push_back({name, std::move(factory)});
    }

    const std::vector<Entry>& all() const { return entries_; }

    std::unique_ptr<T> create(const std::string& name) const {
        for (auto& e : entries_) {
            if (e.name == name) return e.factory();
        }
        return nullptr;
    }

    std::vector<std::string> names() const {
        std::vector<std::string> result;
        for (auto& e : entries_) result.push_back(e.name);
        return result;
    }

private:
    std::vector<Entry> entries_;
};

}  // namespace benchmark
