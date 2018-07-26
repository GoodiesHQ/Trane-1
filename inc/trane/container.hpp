#ifndef TRANE_CONTAINER_HPP
#define TRANE_CONTAINER_HPP

#include "random.hpp"
#include "utils.hpp"

#include <iostream>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace trane
{
    template<typename T>
    class Container
    {
    public:
        Container();

        std::unordered_map<uint64_t, std::shared_ptr<T>>& entries();
        const std::unordered_map<uint64_t, std::shared_ptr<T>>& entries() const;
        trane::Random<std::mt19937_64>& random();

        uint64_t add(std::shared_ptr<T>& ptr);
        std::shared_ptr<T> get(uint64_t id);
        void del(uint64_t id);

    private:
        std::mutex m_mu;
        std::unordered_map<uint64_t, std::shared_ptr<T>> m_entries;
        trane::Random<std::mt19937_64> m_random;
    };
}


template<typename T>
trane::Container<T>::Container()
{
    this->m_random.seed();
}


template<typename T>
std::unordered_map<uint64_t, std::shared_ptr<T>>& trane::Container<T>::entries()
{
    return this->m_entries;
}


template<typename T>
const std::unordered_map<uint64_t, std::shared_ptr<T>>& trane::Container<T>::entries() const
{
    return this->m_entries;
}


template<typename T>
trane::Random<std::mt19937_64>& trane::Container<T>::random()
{
    return m_random;
}


template<typename T>
uint64_t trane::Container<T>::add(std::shared_ptr<T>& ptr)
{
    SCOPELOCK(m_mu);
    uint64_t id;
    do
    {
        id = this->m_random.gen();
    }while(m_entries.find(id) != m_entries.end());
    m_entries[id] = ptr;
    return id;
}


template<typename T>
std::shared_ptr<T> trane::Container<T>::get(uint64_t id)
{
    SCOPELOCK(m_mu);
    auto entry = m_entries.find(id);
    if(entry == m_entries.end())
    {
        return nullptr;
    }
    return entry.second;
}


template<typename T>
void trane::Container<T>::del(uint64_t id)
{
    SCOPELOCK(m_mu);
    m_entries.erase(id);
}

#endif
