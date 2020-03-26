#pragma once

#include "peer/slang.hpp"
#include "slang/port.hpp"

#include "asn/der.hpp"

namespace WarGrey::DTPM {
	private enum class GP : unsigned int {// order matters
		Speed, Track, Heading, BowDirection, TurnRate,

		Latitude, Longitude, Altitude, GeoX, GeoY,

		_
	};

	private class DGPS : public WarGrey::GYDM::IASNSequence {
	public:
		DGPS();
		DGPS(const uint8* basn, size_t* offset = nullptr);

	public:
		void set(WarGrey::DTPM::GP field, double value);
		double ref(WarGrey::DTPM::GP field);

	protected:
		size_t field_payload_span(size_t idx) override;
		size_t fill_field(size_t idx, uint8* octets, size_t offset);
		void extract_field(size_t idx, const uint8* basn, size_t* offset);

	private:
		double metrics[_N(GP)];
	};

	/*********************************************************************************************/
	unsigned short dgps_slang_port(WarGrey::GYDM::SlangPort sp);
	
	void dgps_slang_teardown();
	
	WarGrey::GYDM::ISlangDaemon* dgps_slang_ref(WarGrey::GYDM::SlangPort sp);
}
