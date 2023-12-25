#pragma once
#include <map>
#include <thread>
#include <Nucleona/language.hpp>
#include <mutex>
#include <memory>
namespace cpt {
namespace utility {
template<class V>
struct ThreadDataTable
{
    using TLocalData = V;
    ThreadDataTable() = default;
    DEFAULT_MOVE(ThreadDataTable);
    DISABLE_COPY(ThreadDataTable);
    std::map<std::thread::id, V> storage;
    std::unique_ptr<std::mutex> storage_mux { new std::mutex() };
    template<class... VARGS>
    void allocate_storage(VARGS&&... vargs)
    {
        std::lock_guard<std::mutex> lock(*storage_mux);
        storage.emplace(
              std::this_thread::get_id()
            , std::forward<VARGS>(vargs)...
        );
    }

    GETTER(auto&, get(), 
    {
        auto itr = storage.find( std::this_thread::get_id() );
        if ( itr == storage.end() )
        {
            std::lock_guard<std::mutex> lock(*storage_mux);
            return storage.emplace_hint( itr, std::this_thread::get_id(), V() ) -> second;
        }
        else return itr->second;
        return storage.at(std::this_thread::get_id());
    })

    const auto& look_storage() const
    {
        return storage;
    }
};
}}
