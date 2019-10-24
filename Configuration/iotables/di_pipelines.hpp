#pragma once

#include "graphlet/symbol/pump/water_pumplet.hpp"
namespace WarGrey::SCADA {
	// DB205, starts from 1
	static unsigned int pipeline_L1_W = 2305U;
	static unsigned int pipeline_L1_G = 2306U;
	static unsigned int pipeline_L2_W = 2307U;
	static unsigned int pipeline_L2_G = 2308U;
	static unsigned int pipeline_L3_W = 2309U;

	/************************************************************************************************/
	bool DI_pipeline_ready(const uint8* db205, size_t idx205_p1);
}
