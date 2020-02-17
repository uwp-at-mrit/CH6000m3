#include "brightness.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

/*************************************************************************************************/
Platform::String^ WarGrey::GYDM::brightness_preference_key(BrightnessPort bp) {
    return make_wstring(L"Widget_%s_Brightness_Sync_Key", bp.ToString()->Data());
}

unsigned short WarGrey::GYDM::brightness_slang_port(BrightnessPort bp) {
    unsigned short port = 0;

    switch (bp) {
    case BrightnessPort::SCADA: port = 9132; break;
    case BrightnessPort::DTPM: port = 1112; break;
    }

    return port;
}
