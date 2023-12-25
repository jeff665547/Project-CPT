#pragma once
/**
 * this class will replace SharedObjectManager
 */
#include <CPT/forward.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/format/tsv.hpp>
#include <functional>
#include <map>
#include <CPT/logger.hpp>
#include <cassert>
#include <Nucleona/language.hpp>
#include <CPT/utility.hpp>
// #include <Nucleona/type_traits/is_callable.hpp>
#include <typeindex>

namespace cpt {
namespace engine {
namespace data_pool {
namespace share_data_detail {

template<class T>
class SharedObj
{
    T**             p_data_     ;
    std::size_t*    p_rc_       ;

  public:
    SharedObj()
    : p_data_   ( new T*(nullptr)       )
    , p_rc_     ( new std::size_t(1)    )
    {}
    SharedObj( const SharedObj& obj )
    : p_data_   ( obj.p_data_           )
    , p_rc_     ( obj.p_rc_             )
    {
        (*p_rc_)++;
    }
    SharedObj( SharedObj&& obj )
    : p_data_   ( std::move(obj.p_data_) )
    , p_rc_     ( std::move(obj.p_rc_  ) )
    {}
    SharedObj& operator= ( const SharedObj& obj )
    {
        release();
        p_data_  =  obj.p_data_ ;          
        p_rc_    =  obj.p_rc_   ;          
        (*p_rc_)++;
        return *this;
    }
    SharedObj& operator= ( SharedObj&& obj )
    {
        release();
        p_data_  =  std::move( obj.p_data_) ;          
        p_rc_    =  std::move( obj.p_rc_  ) ;          
        return *this;
    }
#define LOAD_INST(exp) \
    release(); \
    *p_data_= new T exp ;
#define F_LOAD(exp) LOAD_INST(exp);
    template<class LF>
    T& load_f( LF&& lf )
    {
        F_LOAD( (lf()) );
        return **p_data_;
    }
    T& load_o()
    {
        F_LOAD( () );
        return **p_data_;
    }
    template<class O>
    T& load_o( O&& o )
    {
        F_LOAD( (std::forward<O>(o)) );
        return **p_data_;
    }
#undef LOAD_INST
#undef F_LOAD
    bool is_load() { return *p_data_ != nullptr; }
    const std::size_t& rc()
    {
        assert(p_rc_ != nullptr);
        return *p_rc_;
    }
    GETTER(auto&, get(), 
    {
        return **p_data_;
    });
    inline void release()
    {
        auto& rc = *p_rc_;
        if     ( rc  > 0 ) rc --;
        else if( rc == 0 )
        {
            delete *p_data_;
        }
        else throw std::logic_error("impossible rc");
    }
    ~SharedObj()
    {
        release();
        if( *p_rc_ == 0 )
        {
            delete p_data_;
            p_data_ = nullptr;
            delete p_rc_;
            p_rc_ = nullptr;
        }
    }
};
}

template<class T>
struct SharedObjIns
{
    std::size_t* const rc   { new std::size_t(0) };
    T* d                    { nullptr            };
    auto release()
    {
        auto& drc = *rc;
        if ( drc > 0 ) drc --;
        if ( drc == 0 ) 
        {
            if ( d != nullptr ) 
            {
                delete d;
                d = nullptr;
            }
            return true;
        }
        else return false;
    }
    void load( T* const p )
    {
        d = p;
    }
    ~SharedObjIns()
    {
        release();
        delete rc;
    }
};
template<class T>
struct SharedObjectPoolMeta
{
    static std::map<
          std::type_index
        , T
    > events_pool;
};
#ifdef SINGLE_CPP
template<class T>
std::map<std::type_index, T> SharedObjectPoolMeta<T>::events_pool;
#endif

using ObjectsAddrGetter = SharedObjectPoolMeta<
    std::map<
          std::string // name
        , std::function<void*(void)> // addr getter
    >
>;

template<class T>
struct SharedObjectPool
{
  private:
    static std::map<std::string, SharedObjIns<T>> objects;
  public:
    static auto& get_object(const std::string& name)
    {
        auto& object = objects[name];
        auto& addr_getter = ObjectsAddrGetter::events_pool[typeid(T)][name];
        if ( !addr_getter ) 
            addr_getter = [name]()
            {
                return (void*)(SharedObjectPool<T>::objects.at(name).d);
            };
        return object;
    }
    static void clear()
    {
        cpt::clear(objects);
    }
    static bool soft_bind( const std::string& name, SharedObjIns<T>& s )
    {
        auto&& p ( objects.emplace( name, s ));
        return p.second;
    }
    static auto size()
    {
        return objects.size();
    }
    static auto release(const std::string& name )
    {
        return objects.erase(name);
    }
};
#ifdef SINGLE_CPP
template<class T> 
std::map<std::string, SharedObjIns<T>> SharedObjectPool<T>::objects;
#endif
template<class T>
struct SharedObjImg
{ 
    std::string name;
    SharedObjIns<T>* instance { nullptr };

    template<class STR>
    SharedObjImg( STR&& str )
    // : name      ( std::forward<STR>(str) )
    // , instance  ( &SharedObjectPool<T>::get_object(name) )
    {
        // (*(instance->rc)) ++;
        require( std::forward<STR>(str) );
    }
    SharedObjImg(){}
    DISABLE_COPY(SharedObjImg);
    DISABLE_MOVE(SharedObjImg);

    template<class STR>
    void require( STR&& _name )
    {
        name = std::forward<STR>(_name);
        instance = &SharedObjectPool<T>::get_object(name);
        (*(instance->rc)) ++;
    }

    T& get() 
    {
        return *(instance->d);
    }
    auto release()
    {
        return instance->release();
    }
    const std::size_t& rc()
    {
        return *(instance->rc);
    }
    void load_check()
    {
        if ( instance->d != nullptr )
        {
            throw std::logic_error( 
                "double load of object, name : " 
                + name 
            );
        }
    }
    bool is_load()
    {
        return instance->d != nullptr;
    }
    template<class LF>
    T& load_f( LF&& lf )
    {
        load_check();
        instance->load(new T(lf()));
        return *(instance->d);
    }
    template<class... O>
    T& load_o( O&&... o )
    {
        load_check();
        instance->load(new T(std::forward<O>(o)...));
        return *(instance->d);
    }

    ~SharedObjImg()
    {
        if ( release() )
        {
            // SharedObjectPool<T>::release(name); 
        }
    }
};
template<class T>
using Shared = SharedObjImg<T>;

struct SharedDataManager
{
  private:
    std::vector<std::function<void(void)>> initialize_funcs;

  public:
    template <class T>
    auto require(const std::string& name)
    {
        Shared<T> res(name);
        return res;
    }
    template<class T>
    auto require( Shared<T>& s, const std::string& name )
    {
        s.require(name);
    }
    template<class T>
    auto hard_bind( const std::string& name, Shared<T>& s )
    {
        SharedObjectPool<T>::get_object(name) = s;
    }
    template<class T>
    bool soft_bind( const std::string& name, Shared<T>& s )
    {
        return SharedObjectPool<T>::soft_bind(name, s);
    }
    template<class T, class...O>
    auto hard_load   ( Shared<T>& s, O&&... o ) const /* load anyway */
    {
        return s.load_o(std::forward<O>(o)...);
    }
    template<class T, class...O>
    auto hard_load_f ( Shared<T>& s, O&&... o ) const /* load anyway */
    {
        return s.load_f(std::forward<O>(o)...);
    }
    template<class T, class...O>
    auto soft_load   ( Shared<T>& s, O&&... o ) const 
    /* if the data is not exist then do the the data loading otherwise do nothing */
    {
        if( !s.is_load() )
        {
            s.load_o(std::forward<O>(o)...);
        }
        return s.get();
    }
    template<class T, class...O>
    auto soft_load_f ( Shared<T>& s, O&&... o ) const 
    /* if the data is not exist then do the the data loading otherwise do nothing */
    {
        if( !s.is_load() )
        {
            s.load_f(std::forward<O>(o)...);
        }
        return s.get();
    }
    template<class T>
    void release( Shared<T>& s) const
    {
        s.release();
    }
    void initial_shared_data_manager(void)
    {
        for (auto& run: initialize_funcs)
            run();
    }
    template<class OS>
    void dump_objects_addr( OS&& os )
    {
        for ( auto&& t : ObjectsAddrGetter::events_pool )
        {
            os << "type : " << t.first.name() << std::endl;
            for ( auto&& na : t.second )
            {
                os << "   " << na.first << ", " << na.second() << std::endl;
            }
        }
    }
    // template <class T>
    // auto cross_require(Shared<T>& s, const std::string& name1, const std::string& name2 )
    // {
    //     auto& objptr1 ( SharedObjectPool<T>::symbol_table[name1] );
    //     auto& objptr2 ( SharedObjectPool<T>::symbol_table[name2] );
    //     
    //     if( objptr1.is_null() && !objptr2.is_null() )
    //     {
    //         objptr1 = objptr2;
    //         s = (*objptr2.p);
    //     }
    //     else if ( !objptr1.is_null() && objptr2.is_null() )
    //     {
    //         objptr2 = objptr1;
    //         s = (*objptr1.p);
    //     }
    //     else if ( objptr1.is_null() && objptr2.is_null() )
    //     {
    //         require(s, name1);
    //         objptr1 = &s;
    //         objptr2 = &s;
    //     }
    //     else
    //     {
    //         throw std::logic_error("bad cross require");
    //     }
    // }
    
};
}}}
