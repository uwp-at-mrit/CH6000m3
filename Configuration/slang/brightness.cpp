#include "slang/brightness.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;

/*************************************************************************************************/
Platform::String^ WarGrey::GYDM::brightness_preference_key(SlangPort sp) {
    return make_wstring(L"Widget_%s_Brightness_Sync_Key", sp.ToString()->Data());
}

unsigned short WarGrey::GYDM::brightness_slang_port(SlangPort sp) {
    unsigned short port = 0;

    switch (sp) {
    case SlangPort::SCADA: port = 9132; break;
    case SlangPort::DTPM: port = 1112; break;
    }

    return port;
}
