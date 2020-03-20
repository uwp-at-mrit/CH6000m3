#include <map>

#include "slang/dgps.hpp"
#include "configuration.hpp"

#include "datum/flonum.hpp"

#include "syslog.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

/*************************************************************************************************/
static std::map<SlangPort, ISlangDaemon*> slangds;

static ISlangDaemon* setup_slangd(SlangPort sp) {
	Syslog* slang_logger = make_system_logger(default_slang_logging_level, sp.ToString());
	ISlangDaemon* slangd = new SlangDaemon<uint8>(slang_logger, dgps_slang_port(sp));

	slangd->join_multicast_group(slang_multicast_group);

	slangds.insert(std::pair<SlangPort, ISlangDaemon*>(sp, slangd));

	return slangd;
}

/*************************************************************************************************/
DGPS::DGPS() : IASNSequence(_N(GP)) {}

void DGPS::set(GP field, double v) {
	if (field < GP::_) {
		this->metrics[_I(field)] = v;
	}
}

double DGPS::ref(GP field) {
	return ((field < GP::_) ? this->metrics[_I(field)] : flnan);
}

DGPS::DGPS(const uint8* basn, size_t* offset) : DGPS() {
	this->from_octets(basn, offset);
}

size_t DGPS::field_payload_span(size_t idx) {
	return asn_real_span(this->metrics[idx]);
}

size_t DGPS::fill_field(size_t idx, uint8* octets, size_t offset) {
	return asn_real_into_octets(this->metrics[idx], octets, offset);
}

void DGPS::extract_field(size_t idx, const uint8* basn, size_t* offset) {
	this->metrics[idx] = asn_octets_to_real(basn, offset);
}

/*************************************************************************************************/
unsigned short WarGrey::DTPM::dgps_slang_port(SlangPort sp) {
	unsigned short port = 0;

	switch (sp) {
	case SlangPort::SCADA: port = 7796; break;
	case SlangPort::DTPM: port = 5797; break;
	}

	return port;
}

void WarGrey::DTPM::dgps_slang_teardown() {
	for (auto it = slangds.begin(); it != slangds.end(); it++) {
		delete it->second;
	}

	slangds.clear();
}

ISlangDaemon* WarGrey::DTPM::dgps_slang_ref(SlangPort sp) {
	if (slangds.find(sp) == slangds.end()) {
		setup_slangd(sp);
	}

	return slangds[sp];
}
