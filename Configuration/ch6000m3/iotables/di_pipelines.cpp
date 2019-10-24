#include "ch6000m3/plc.hpp"
#include "ch6000m3/iotables/di_pipelines.hpp"

using namespace WarGrey::SCADA;

/**************************************************************************************************/
bool WarGrey::SCADA::DI_pipeline_ready(const uint8* db205, size_t idx205_p1) {
	return DBX(db205, idx205_p1 - 1U);
}
