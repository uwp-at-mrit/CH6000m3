#pragma once

#include "slang/port.hpp"

#include "asn/der.hpp"

namespace WarGrey::GYDM {
	private class DGPS : public WarGrey::GYDM::IASNSequence {

	};

	unsigned short dgps_slang_port(WarGrey::GYDM::SlangPort sp);
}
