#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include "../ast/Nodes.hpp"

namespace miniml {

    /// A simple lexical environment stack: each frame is a map name -> definition location.
    /// The back() frame is the innermost scope.
    class EnvStack {
    public:
        EnvStack() { push(); }                    // start with a global frame

        // Scope management
        void push() { scopes_.emplace_back(); }
        void pop()  { if (!scopes_.empty()) scopes_.pop_back(); }

        // Bind a name in the current (innermost) scope. Returns the previous position if it shadowed an outer binding.
        std::optional<SrcLoc> bind(const std::string& name, SrcLoc loc) {
            std::optional<SrcLoc> previous;
            for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
                auto f = it->find(name);
                if (f != it->end()) {
                    previous = f->second;
                    break;
                }
            }
            scopes_.back()[name] = loc;
            return previous; // empty if no shadowing
        }

        // Lookup
        bool isBound(const std::string& name) const {
            return static_cast<bool>(lookup(name));
        }

        // Where was it defined? (nearest binding)
        std::optional<SrcLoc> lookup(const std::string& name) const {
            for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
                auto f = it->find(name);
                if (f != it->end()) return f->second;
            }
            return std::nullopt;
        }

        bool bindsInCurrent(const std::string& name) const {
            return scopes_.empty() ? false : scopes_.back().count(name) != 0;
        }

        // Introspection
        size_t depth() const { return scopes_.size(); }

    private:
        // map: name -> definition location
        using Frame = std::unordered_map<std::string, SrcLoc>;
        std::vector<Frame> scopes_;
    };

} // namespace miniml
