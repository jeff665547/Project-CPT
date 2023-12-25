#pragma once
#include <string>
#include <locale>
#include <codecvt>
namespace cpt { namespace format {
    using WStrUTF8 = std::wstring_convert<
        std::codecvt_utf8<wchar_t>
    >;
    using WStrUTF16LE = std::wstring_convert<
        std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>
    >;

}}
