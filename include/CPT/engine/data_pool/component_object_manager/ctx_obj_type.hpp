#pragma once
#include <string>
#include <CPT/utility.hpp>
#include <iostream>
#include <functional>
namespace cpt { namespace engine { namespace data_pool {
namespace component_object_manager {

class CtxObjType
{
  public:
    enum ID { PATH, ADDR, KEY, PATH_LIST };
    ID id;
    std::function<void(void)> destruct_event;
    template<class T>
    void show_info( T&& out, int il = 0 )
    {
        out << cpt::indent(il) << "id : " << [this]() -> std::string
        {
            switch(id)
            {
                case PATH: return "path";
                case ADDR: return "addr";
                case KEY : return "key" ;
                case PATH_LIST : return "path_list";
            }
        }() << std::endl;
    }
    ~CtxObjType()
    {
        if ( destruct_event )
        {
            destruct_event();
            destruct_event = {};
        }
    }

};

}}}}
