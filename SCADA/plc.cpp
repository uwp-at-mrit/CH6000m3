#include "plc.hpp"
#include "box.hpp"
#include "enum.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation::Numerics;

static uint16 MRIT_PORT = 2008;

private enum MRDB {
	REALTIME           = 2,
	FORAT              = 20,
	DIGITAL_INPUT      = 205,
	DIGITAL_INPUT_RAW  = 4,
	DIGITAL_OUTPUT_RAW = 6,
	ANALOG_INPUT       = 203,
	ANALOG_INPUT_RAW   = 3,
	ANALOG_OUTPUT      = 204,
	ANALOG_OUTPUT_RAW  = 5
};

static bool fill_signal_preferences(size_t type, size_t* count, size_t* addr0, size_t* addrn) {
	bool has_set = true;
	size_t c = 0;
	size_t start = 0;
	size_t end = 0;

	switch (type) {
	case MRDB::ANALOG_INPUT_RAW:   c = 280;     start = 0;    end = 1119; break; // DB3
	case MRDB::ANALOG_INPUT:       c = 280;     start = 1120; end = 2239; break; // DB203
	case MRDB::ANALOG_OUTPUT_RAW:  c = 48;      start = 2240; end = 2431; break; // DB5
	case MRDB::ANALOG_OUTPUT:      c = 48;      start = 2432; end = 2623; break; // DB204
	case MRDB::FORAT:              c = 2 + 198; start = 2624; end = 3417; break; // DB20, the first two are DIs
	case MRDB::REALTIME:           c = 176;     start = 3418; end = 4121; break; // DB2

	case MRDB::DIGITAL_INPUT_RAW:  c = 124;     start = 4122; end = 4245; break; // DB4
	case MRDB::DIGITAL_OUTPUT_RAW: c = 76;      start = 4246; end = 4321; break; // DB6
	case MRDB::DIGITAL_INPUT:      c = 385;     start = 4322; end = 4706; break; // DB205

	default: has_set = false; break;
	}

	if (has_set) {
		SET_BOX(count, c);
		SET_BOX(addr0, start);
		SET_BOX(addrn, end);
	}

	return has_set;
}

static bool valid_address(Syslog* logger, size_t db, size_t addr0, size_t addrn, size_t count, size_t unit_size, size_t total) {
	bool valid = ((addr0 + count * unit_size) <= total);

	if (((addrn - addr0 + 1) != (count * unit_size))) {
		logger->log_message(Log::Warning,
			L"the address range [%u, %u] of DB%u is misconfigured or mistyped(given count: %u; expect: %u)",
			addr0, addrn, db, (addrn - addr0 + 1) / unit_size, count);
	}

	if (!valid) {
		logger->log_message(Log::Error,
			L"the end address of DB%u is misconfigured or mistyped(address: %u > %u)",
			db, addrn, total);
	}

	return valid;
}

/*************************************************************************************************/
bool WarGrey::SCADA::DBX(const uint8* src, size_t idx) {
	return DBX(src, idx / 8U, idx % 8U);
}

bool WarGrey::SCADA::DBX(const uint8* src, size_t idx, size_t bidx) {
	return quantity_bit_ref(src, idx, (unsigned char)bidx);
}

float WarGrey::SCADA::DBD(const uint8* src, size_t idx) {
	return bigendian_float_ref(src, idx);
}

float WarGrey::SCADA::RealData(const uint8* src, size_t idx) {
	return bigendian_float_ref(src, idx * 4U);
}

float3 WarGrey::SCADA::DBD_3(const uint8* src, size_t idx) {
	float3 position;

	position.x = DBD(src, idx + 0U);
	position.y = DBD(src, idx + 4U);
	position.z = DBD(src, idx + 8U);

	return position;
}

/*************************************************************************************************/
PLCMaster::PLCMaster(Syslog* logger, long long ms) : MRMaster(logger, MRIT_PORT), last_sent_time(-1L) {
	this->set_suicide_timeout(ms);
}

void PLCMaster::send_scheduled_request(long long count, long long interval, long long uptime) {
	if (this->last_sent_time != uptime) {
		this->read_all_signal((uint16)98U, (uint16)0U, (uint16)0x1263U);
		this->last_sent_time = uptime;
	}
}

void PLCMaster::send_setting(int16 address, float datum) {
	if (address > 0U) {
		this->write_analog_quantity((uint16)20U, address, datum);
	}
}

void PLCMaster::send_command(uint8 idx, uint8 bidx) {
	this->write_digital_quantity((uint16)300U, idx, bidx, true);
}

void PLCMaster::send_command(uint16 index_p1) {
	int16 idx = index_p1 - 1U;

	if (idx >= 0) {
		this->send_command((uint8)(idx / 8), (uint8)(idx % 8));
	}
}

/*************************************************************************************************/
void PLCConfirmation::on_all_signals(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	size_t count, subaddr0, subaddrn;
	size_t adbs[] = { MRDB::REALTIME, MRDB::ANALOG_INPUT, MRDB::FORAT };
	size_t ddbs[] = { MRDB::DIGITAL_INPUT_RAW, MRDB::DIGITAL_INPUT };
	size_t dqcount = 2;
	size_t analog_size = sizeof(float);
	size_t digital_size = sizeof(uint8);
	uint8* digital_data[] = { nullptr, nullptr };
	size_t digital_counts[] = { 0, 0 };
	uint8* analog_data[] = { nullptr, nullptr, nullptr };
	size_t analog_counts[] = { 0, 0, 0 };

	this->pre_read_data(logger);

	for (size_t i = 0; i < sizeof(ddbs) / sizeof(size_t); i++) {
		if (fill_signal_preferences(ddbs[i], &count, &subaddr0, &subaddrn)) {
			if (valid_address(logger, ddbs[i], subaddr0, subaddrn, count, digital_size, size)) {
				digital_data[i] = data + subaddr0;
				digital_counts[i] = count;
			}
		} else {
			logger->log_message(Log::Warning, L"missing configuration for data block %hu", ddbs[i]);
		}
	}

	if ((digital_data[0] != nullptr) && (digital_data[1] != nullptr)) {
		this->on_digital_input(timepoint_ms, digital_data[0], digital_counts[0], digital_data[1], digital_counts[1], logger);
	}

	for (size_t i = 0; i < sizeof(adbs) / sizeof(size_t); i++) {
		if (fill_signal_preferences(adbs[i], &count, &subaddr0, &subaddrn)) {
			if (adbs[i] == MRDB::FORAT) {
				// this is a special case, some digital data is stored in the first two bytes. 
				subaddr0 += dqcount;
				count -= dqcount;
			}

			if (valid_address(logger, adbs[i], subaddr0, subaddrn, count, analog_size, size)) {
				analog_data[i] = data + subaddr0;
				analog_counts[i] = count * analog_size;
			}
		} else {
			logger->log_message(Log::Warning, L"missing configuration for data block %hu", adbs[i]);
		}
	}

	if ((analog_data[0] != nullptr) && (analog_data[1] != nullptr)) {
		this->on_analog_input(timepoint_ms, analog_data[0], analog_counts[0], analog_data[1], analog_counts[1], logger);
	}

	if (analog_data[2] != nullptr) {
		this->on_forat(timepoint_ms, analog_data[2] - dqcount, analog_counts[2] + dqcount, logger);
	}

	this->post_read_data(logger);
}
