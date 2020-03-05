#include "slang/dgps.hpp"

using namespace WarGrey::GYDM;

/*************************************************************************************************/
unsigned short WarGrey::GYDM::dgps_slang_port(SlangPort sp) {
    unsigned short port = 0;

    switch (sp) {
    case SlangPort::SCADA: port = 7796; break;
    case SlangPort::DTPM: port = 5797; break;
    }

    return port;
}
