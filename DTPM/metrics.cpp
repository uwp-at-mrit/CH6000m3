#include "metrics.hpp"

using namespace WarGrey::DTPM;

static ResidentMetrics* resident_mertics = nullptr;

/*************************************************************************************************/
ResidentMetrics::ResidentMetrics() : IASNSequence(2) {}

ResidentMetrics::ResidentMetrics(const uint8* basn, size_t* offset) : ResidentMetrics() {
	this->from_octets(basn, offset);
}

size_t ResidentMetrics::field_payload_span(size_t idx) {
	size_t span = 0;

	switch (idx) {
	case 0: span = this->gps.span(); break;
	case 1: span = this->tp.span(); break;
	}

	return span;
}

size_t ResidentMetrics::fill_field(size_t idx, uint8* octets, size_t offset) {
	switch (idx) {
	case 0: offset = this->gps.into_octets(octets, offset); break;
	case 1: offset = this->tp.into_octets(octets, offset); break;
	}

	return offset;
}

void ResidentMetrics::extract_field(size_t idx, const uint8* basn, size_t* offset) {
	switch (idx) {
	case 0: this->gps.from_octets(basn, offset); break;
	case 1: this->tp.from_octets(basn, offset); break;
	}
}

/*************************************************************************************************/
ResidentMetrics* WarGrey::DTPM::global_resident_metrics() {
    if (resident_mertics == nullptr) {
		resident_mertics = new ResidentMetrics();
    }

    return resident_mertics;
}
