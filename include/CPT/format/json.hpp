#include <Nucleona/format/json.hpp>
namespace cpt::format {

    template<class T>
    using Json = nucleona::format::Json<T>;

    template<class T>
    using JsonList = nucleona::format::JsonList<T>;

    using nucleona::format::make_json;
    using nucleona::format::make_json_list;
    using nucleona::format::read_json;

}