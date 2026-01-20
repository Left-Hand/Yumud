#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

#include "core/string/view/string_view.hpp"
#include "core/utils/Option.hpp"

namespace ymd::fs{

class Path {
private:
    StringView path_;

public:
    // 构造函数
    constexpr Path() noexcept = default;
    
    constexpr explicit Path(StringView path) noexcept 
        : path_(path) {}
    
    constexpr Path(const char* path) noexcept 
        : path_(path) {}
    
    // 拷贝构造函数
    constexpr Path(const Path&) noexcept = default;
    
    // 移动构造函数
    constexpr Path(Path&&) noexcept = default;
    
    // 赋值运算符
    constexpr Path& operator=(const Path&) noexcept = default;
    constexpr Path& operator=(Path&&) noexcept = default;
    
    // 判断是否为空
    constexpr bool is_empty() const noexcept {
        return path_.is_empty();
    }
    
    // 转换为字符串视图
    constexpr StringView as_str() const noexcept {
        return path_;
    }
    
    // 判断是否为绝对路径（简单实现，不依赖平台）
    constexpr bool is_absolute() const noexcept {
        if (path_.is_empty()) return false;
        
        // Unix风格绝对路径
        if (path_[0] == '/') return true;
        
        // Windows风格绝对路径（简化处理）
        if (path_.size() >= 3 && 
                ((path_[0] >= 'a' && path_[0] <= 'z') || 
                (path_[0] >= 'A' && path_[0] <= 'Z')) &&
            path_[1] == ':' && 
            (path_[2] == '/' || path_[2] == '\\')) {
            return true;
        }
        
        return false;
    }
    
    // 判断是否为相对路径
    constexpr bool is_relative() const noexcept {
        return !is_absolute();
    }
    
    // 获取父路径
    constexpr Option<Path> parent() const noexcept {
        if (path_.is_empty()) return None;
        
        // 查找最后一个分隔符
        std::size_t last_sep = path_.find_last_of("/\\");
        if (last_sep == StringView::npos) {
            // 没有分隔符，返回空路径或当前目录
            if (path_ == "." || path_ == "..") {
                return Some{Path(".")};
            }
            return Some{Path("")};
        }
        
        // 处理连续分隔符的情况
        while (last_sep > 0 && 
                (path_[last_sep - 1] == '/' || path_[last_sep - 1] == '\\')) {
            --last_sep;
        }
        
        if (last_sep == 0) {
            // 根目录的情况
            return Some{Path(path_.substr_by_range(0, 1))};
        }
        
        return Some{Path(path_.substr_by_range(0, last_sep))};
    }
    
    // 获取文件名部分
    constexpr Option<StringView> file_name() const noexcept {
        if (path_.is_empty()) return Option<StringView>{};
        
        // 查找最后一个分隔符
        std::size_t last_sep = path_.find_last_of("/\\");
        if (last_sep == StringView::npos) {
            return Option<StringView>{path_};
        }
        
        if (last_sep + 1 >= path_.size()) {
            return Option<StringView>{};
        }
        
        return Option<StringView>{path_.substr(last_sep + 1)};
    }
    
    // 获取扩展名
    constexpr Option<StringView> extension() const noexcept {
        auto filename_opt = file_name();
        if (!filename_opt.is_some()) {
            return None;
        }
        
        StringView filename = filename_opt.unwrap();
        std::size_t dot_pos = filename.find_last_of('.');
        
        if (dot_pos == StringView::npos || 
            dot_pos == 0 || 
            dot_pos == filename.size() - 1) {
            return None;
        }
        
        return Option(filename.substr(dot_pos + 1));
    }
    
    // 判断是否有扩展名
    constexpr bool has_extension() const noexcept {
        return extension().is_some();
    }
    
    // 路径连接
    constexpr Path join(StringView other) const noexcept {
        if (path_.is_empty()) return Path(other);
        if (other.is_empty()) return *this;
        
        // 检查是否需要添加分隔符
        bool needs_sep = !path_.is_empty() && 
                        path_.back() != '/' && 
                        path_.back() != '\\' &&
                        other.front() != '/' && 
                        other.front() != '\\';
        
        if (needs_sep) {
            // 简单的连接实现，使用Unix风格分隔符
            return Path(StringView(path_.data(), path_.size()) + "/" + other);
        }
        
        return Path(StringView(path_.data(), path_.size()) + other);
    }
    
    constexpr Path join(const Path& other) const noexcept {
        return join(other.as_str());
    }
    
    // 路径比较
    constexpr bool operator==(const Path& other) const noexcept {
        return path_ == other.path_;
    }
    
    constexpr bool operator!=(const Path& other) const noexcept {
        return !(*this == other);
    }
    
    // 转换为字符串（返回视图）
    constexpr operator StringView() const noexcept {
        return path_;
    }
    
    // 检查路径是否以某个前缀开头
    constexpr bool starts_with(const Path& prefix) const noexcept {
        if (prefix.path_.size() > path_.size()) return false;
        
        // 简单的前缀比较
        return path_.substr_by_range(0, prefix.path_.size()) == prefix.path_;
    }
    
    // 检查路径是否以某个后缀结尾
    constexpr bool ends_with(const Path& suffix) const noexcept {
        if (suffix.path_.size() > path_.size()) return false;
        
        return path_.substr(path_.size() - suffix.path_.size()) == suffix.path_;
    }
    
    // 规范化路径（简化实现）
    constexpr Path normalize() const noexcept {
        if (path_.is_empty()) return *this;
        
        // 简单的规范化：移除"./"前缀，处理"../"
        // 注意：这是一个简化实现，不处理复杂的路径规范化
        
        StringView result = path_;
        
        // 移除开头的"./"
        if (result.starts_with("./")) {
            result = result.substr(2);
        }
        
        // 简单的处理"../"
        // 这里只是演示，实际实现会更复杂
        
        return Path(result);
    }
};

// 连接操作符重载
constexpr Path operator/(const Path& lhs, const Path& rhs) noexcept {
    return lhs.join(rhs);
}

constexpr Path operator/(const Path& lhs, StringView rhs) noexcept {
    return lhs.join(rhs);
}

constexpr Path operator/(StringView lhs, const Path& rhs) noexcept {
    return Path(lhs).join(rhs);
}

// 一些工具函数
namespace path {
    constexpr bool is_separator(char c) noexcept {
        return c == '/' || c == '\\';
    }
    
    constexpr bool has_root_component(StringView path) noexcept {
        return !path.is_empty() && is_separator(path[0]);
    }
}

}