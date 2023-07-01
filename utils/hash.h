#ifndef _HASH_H
#define _HASH_H

#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <set>

namespace UTILS
{
    class Hash
    {
    private:
        std::hash<std::string> hash_str;
    public:
        size_t operator()(const std::string &str) const
        {
            return hash_str(str);
        }

        template <typename _T>
        size_t operator()(const std::set<_T> &vec) const
        {
            return hash_container(vec);
        }
        
        template <typename _T>
        size_t operator()(const std::vector<_T> &vec) const
        {
            return hash_container(vec);
        }

        size_t operator()(const std::initializer_list<size_t> &vec) const
        {
            return hash_container(vec);
        }

    private:
        template <typename _Container>
        size_t hash_container(const _Container &vec) const
        {
            size_t _hash = 0;
            for (const auto &elem : vec)
            {
                combine(_hash, elem);
            }
            return _hash;
        }

        template <typename T>
        void combine(std::size_t &seed, const T &value) const
        {
            seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
    };

    template<typename _T>
    size_t hash(_T& term)
    {
        return term.__hash__();
    }
} // namespace UTILS

#endif // _HASH_H