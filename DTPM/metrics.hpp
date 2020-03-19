#pragma once

#include "slang/dgps.hpp"
#include "metrics/times.hpp"

#include "asn/der.hpp"

namespace WarGrey::DTPM {
	private enum class MetricsBlock : unsigned int { Default };

	private class ResidentMetrics : public WarGrey::GYDM::IASNSequence {
	public:
		ResidentMetrics();
		ResidentMetrics(const uint8* basn, size_t* offse = nullptr);

	public:
		WarGrey::DTPM::DGPS gps;
		WarGrey::DTPM::Timepoint tp;

	protected:
		size_t field_payload_span(size_t idx) override;
		size_t fill_field(size_t idx, uint8* octets, size_t offset);
		void extract_field(size_t idx, const uint8* basn, size_t* offset);
	};

	WarGrey::DTPM::ResidentMetrics* global_resident_metrics();
}
