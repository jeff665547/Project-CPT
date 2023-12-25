#pragma once
#include <memory>
#include <CPT/format/json.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/component_object_manager/detail.hpp>
#include <CPT/engine/data_pool/component_object_manager/ctx_obj_type.hpp>
#include <CPT/engine/data_pool/component_object_manager/cpsym_tab.hpp>
#include <CPT/engine/data_pool/component_object_manager/detail/type_resolver.hpp>
#include <CPT/logger.hpp>
namespace cpt { namespace engine { namespace data_pool {
namespace component_object_manager {

template<class T> class ICPObj;
template<class T> using ICPObjPtr = std::unique_ptr<ICPObj<T>>;

template<class T>
class ICPObj
{
  public:
    virtual T& get() = 0;
    virtual const T& get() const = 0;
    virtual const void* get_addr() const = 0;
    virtual void release() = 0;
    virtual void initialize() = 0;
    virtual void initialize(T&&) = 0;
    virtual void initialize_or(T&&) = 0;
    virtual void show_info(cpt::logger::OpenLog&, int i = 0) = 0; 
    virtual void show_info(cpt::logger::CloseLog&, int i = 0) = 0; 
    virtual void show_info(std::ostream&, int i = 0) = 0; 
    virtual ~ICPObj(){}
};
template<class T>
class LiteralCPObj : public ICPObj<T>
{
    T data_;
    template<class LOG>
    static std::string data_to_string( const std::string& data, LOG& log )
    {
        return data;
    }
    template<class DATA, class LOG>
    static std::string data_to_string( const DATA& data, LOG& log )
    {
        return "";
    }
    template<class LOG>
    void show_info_( LOG& log, int il)
    {
        log << cpt::indent(il) << "object info : " << std::endl;
        log << cpt::indent(il) << "  meta type : literal" << std::endl;
        log << cpt::indent(il) << "       data : " << data_to_string(data_, log) <<  std::endl;
    }
  public: 
    LiteralCPObj( const std::string& content )
    : data_ ( cpt::utility::PathToObj<T>::get( content ) )
    {}
    LiteralCPObj()
    {}
    virtual       T&    get()       override { return data_;  }
    virtual const T&    get() const override { return data_;  }
    virtual const void* get_addr() const override { return &data_; }
    virtual void release() override
    {
        T tmp = std::move(data_);
    }
    virtual void initialize() override
    {}
    virtual void initialize(T&& o) override
    {
        data_ = std::move(o);
    }
    virtual void initialize_or( T&& o ) override
    {
    }
    virtual ~LiteralCPObj()
    {
        this->release();
    }
    virtual void show_info(cpt::logger::OpenLog& log, int il) override
    {
        show_info_(log, il); 
    } 
    virtual void show_info(cpt::logger::CloseLog& log, int il ) override
    {
        show_info_( log, il);
    } 
    virtual void show_info(std::ostream& log, int il) override
    {
        show_info_( log, il );
    }
};
template<class T>
class RefCPObj : public ICPObj<T>
{
    template<class LOG>
    void show_info_( LOG& log, int il )
    {
        log << cpt::indent(il) << "object info : " << std::endl;
        log << cpt::indent(il) << "  meta type : ref" << std::endl;
        log << cpt::indent(il) << "  ref count : " << obj_ref.use_count() << std::endl;
        log << cpt::indent(il) << "       data : "  <<  std::endl;
        obj_ref->show_info(log, il + 1);

    }
    std::shared_ptr<CPSymTabIns> obj_ref;
  public:
    RefCPObj ( const std::string& ref_name )
    : obj_ref ( CPSymTab::get_entry( ref_name ))
    {}
    virtual void initialize( T&& o) override
    {
        obj_ref->initialize(std::move(o));
    }
    virtual       T& get()       override 
    {  
        return obj_ref->get<T>();
    }
    virtual const T& get() const override 
    { 
        return obj_ref->get<T>();
    }
    virtual const void* get_addr() const override 
    {
        return obj_ref->data;
    }
    virtual void initialize() override
    {
        obj_ref->initialize<T>();
    }
    virtual void initialize_or( T&& o ) override
    {
        obj_ref->initialize_or(std::move(o));
    }
    virtual void release() override
    {
        obj_ref.reset();
    }
    virtual ~RefCPObj()
    {
        this->release();
    }
    virtual void show_info(cpt::logger::OpenLog& log, int il) override
    {
        show_info_(log, il); 
    } 
    virtual void show_info(cpt::logger::CloseLog& log, int il ) override
    {
        show_info_( log, il );
    } 
    virtual void show_info(std::ostream& log, int il ) override
    {
        show_info_( log, il );
    }
};
template<class T, class PT>
ICPObjPtr<T> require( const cpt::format::Json<PT>& node )
{
    /* if empty node input, then return local literal
     * the empty node usally means component parameter ( object ) 
     * not specified in pipeline config file, 
     * so the component pass a empty node to this function
     **/
    if ( node.root.data().empty() && node.root.empty() )     
    {
        return ICPObjPtr<T> ( new LiteralCPObj<T>() );
    }

    std::string type = node.template get<std::string>("type");
    std::string content = node.template get<std::string>("content");
    if ( "literal" == type )
    {
        return ICPObjPtr<T> ( new LiteralCPObj<T>(content) );
    }
    else if ( "ref" == type )
    {
        return ICPObjPtr<T> ( new RefCPObj<T>(content) );
    }
    else
    {
        throw std::logic_error("unknown type name : " + type);
    }
}
template<class T, class PT, class DFALT>
ICPObjPtr<T> require_w( const cpt::format::Json<PT>& args, const std::string& label, const DFALT& dfalt )
{
    return require<T>(
        cpt::format::make_json( args.get_child_optional(label).value_or(dfalt) )
    );
}
template<class STR>
auto make_ref_parameter(STR&& content)
{
    auto j ( cpt::format::make_json() );
    j.add("type", "ref");
    j.add("content", content);
    return j.root;
}
}}}}
