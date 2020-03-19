#pragma once

#include "peer/slang.hpp"
#include "slang/port.hpp"

#include "asn/der.hpp"

namespace WarGrey::DTPM {
	private class DGPS : public WarGrey::GYDM::IASNSequence {
	public:
		DGPS();
		DGPS(const uint8* basn, size_t* offse = nullptr);

	public:
		double speed;
		double heading_deg;

		double latitude;
		double longitude;
		double altitude;
		double geo_x;
		double geo_y;

	protected:
		size_t field_payload_span(size_t idx) override;
		size_t fill_field(size_t idx, uint8* octets, size_t offset);
		void extract_field(size_t idx, const uint8* basn, size_t* offset);
	};

	/*********************************************************************************************/
	unsigned short dgps_slang_port(WarGrey::GYDM::SlangPort sp);
	
	void dgps_slang_teardown();
	
	WarGrey::GYDM::ISlangDaemon* dgps_slang_ref(WarGrey::GYDM::SlangPort sp);
}
