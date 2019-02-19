#pragma once

#include "graphlet/symbol/valve/gate_valvelet.hpp"
#include "graphlet/symbol/valve/manual_valvelet.hpp"
#include "graphlet/symbol/valve/tagged_valvelet.hpp"

namespace WarGrey::SCADA {
	// DB4, starts from 1
	static unsigned int manual_valve_SQ1_status = 113U;
	static unsigned int manual_valve_SQ2_status = 114U;

	static unsigned int manual_valve_SQk1_status = 122U;
	static unsigned int manual_valve_SQk2_status = 123U;
	static unsigned int manual_valve_SQl_status = 124U;
	static unsigned int manual_valve_SQm_status = 125U;
	static unsigned int manual_valve_SQy_status = 126U;

	static unsigned int manual_valve_SQi_status = 128U;
	static unsigned int manual_valve_SQj_status = 129U;

	static unsigned int manual_valve_SQc_status = 135U;
	static unsigned int manual_valve_SQd_status = 136U;
	static unsigned int manual_valve_SQe_status = 137U;
	static unsigned int manual_valve_SQf_status = 138U;

	static unsigned int manual_valve_SQa_status = 141U;
	static unsigned int manual_valve_SQb_status = 142U;
	static unsigned int manual_valve_SQg_status = 143U;
	static unsigned int manual_valve_SQh_status = 144U;

	static unsigned int gate_valve_D01_feedback = 279U;
	static unsigned int gate_valve_D02_feedback = 273U;
	static unsigned int gate_valve_D05_feedback = 259U;
	static unsigned int gate_valve_D06_feedback = 261U;
	static unsigned int gate_valve_D07_feedback = 289U;
	static unsigned int gate_valve_D08_feedback = 291U;
	static unsigned int gate_valve_D09_feedback = 293U;
	static unsigned int gate_valve_D10_feedback = 295U;
	static unsigned int gate_valve_D11_feedback = 349U;
	static unsigned int gate_valve_D12_feedback = 333U;
	static unsigned int gate_valve_D13_feedback = 405U;
	static unsigned int gate_valve_D14_feedback = 373U;
	static unsigned int gate_valve_D15_feedback = 407U;
	static unsigned int gate_valve_D16_feedback = 375U;
	static unsigned int gate_valve_D17_feedback = 297U;
	static unsigned int gate_valve_D18_feedback = 299U;
	static unsigned int gate_valve_D19_feedback = 301U;
	static unsigned int gate_valve_D20_feedback = 303U;
	static unsigned int gate_valve_D21_feedback = 305U;
	static unsigned int gate_valve_D22_feedback = 307U;
	static unsigned int gate_valve_D23_feedback = 309U;
	static unsigned int gate_valve_D24_feedback = 413U;
	static unsigned int gate_valve_D25_feedback = 275U;
	static unsigned int gate_valve_D26_feedback = 277U;

	static unsigned int motor_valve_D01_feedback = 465U;
	static unsigned int motor_valve_D02_feedback = 445U;
	static unsigned int motor_valve_D03_feedback = 419U;
	static unsigned int motor_valve_D04_feedback = 449U;
	static unsigned int motor_valve_D05_feedback = 447U;
	static unsigned int motor_valve_D06_feedback = 461U;
	static unsigned int motor_valve_D07_feedback = 423U;
	static unsigned int motor_valve_D08_feedback = 417U;
	static unsigned int motor_valve_D09_feedback = 439U;
	static unsigned int motor_valve_D10_feedback = 467U;
	static unsigned int motor_valve_D11_feedback = 441U;
	static unsigned int motor_valve_D12_feedback = 437U;
	static unsigned int motor_valve_D13_feedback = 433U;
	static unsigned int motor_valve_D14_feedback = 431U;
	static unsigned int motor_valve_D15_feedback = 435U;
	static unsigned int motor_valve_D16_feedback = 427U;
	static unsigned int motor_valve_D17_feedback = 443U;
	static unsigned int motor_valve_D18_feedback = 463U;
	static unsigned int motor_valve_D19_feedback = 453U;
	static unsigned int motor_valve_D20_feedback = 451U;
	static unsigned int motor_valve_D21_feedback = 421U;
	static unsigned int motor_valve_D22_feedback = 459U;
	static unsigned int motor_valve_D23_feedback = 425U;
	static unsigned int motor_valve_D24_feedback = 429U;
	static unsigned int motor_valve_D25_feedback = 455U;
	static unsigned int motor_valve_D26_feedback = 457U;

	static unsigned int butterfly_valve_HBV01_feedback = 281U;
	static unsigned int butterfly_valve_HBV02_feedback = 265U;
	static unsigned int butterfly_valve_HBV03_feedback = 263U;
	static unsigned int butterfly_valve_HBV04_feedback = 283U;
	static unsigned int butterfly_valve_HBV05_feedback = 267U;
	static unsigned int butterfly_valve_HBV06_feedback = 285U;
	static unsigned int butterfly_valve_HBV07_feedback = 269U;
	static unsigned int butterfly_valve_HBV08_feedback = 271U;
	static unsigned int butterfly_valve_HBV09_feedback = 287U;
	static unsigned int butterfly_valve_HBV10_feedback = 241U;
	static unsigned int butterfly_valve_HBV11_feedback = 243U;
	static unsigned int butterfly_valve_HBV12_feedback = 245U;
	static unsigned int butterfly_valve_HBV13_feedback = 247U;
	static unsigned int butterfly_valve_HBV14_feedback = 249U;
	static unsigned int butterfly_valve_HBV15_feedback = 251U;
	static unsigned int butterfly_valve_HBV16_feedback = 253U;
	static unsigned int butterfly_valve_HBV17_feedback = 255U;
	static unsigned int butterfly_valve_HBV18_feedback = 239U;

	// DB205, starts from 1
	static unsigned int gate_valve_D03_feedback = 1645U;
	static unsigned int gate_valve_D04_feedback = 1629U;

	static unsigned int gate_valve_D01_status = 369U;
	static unsigned int gate_valve_D02_status = 393U;
	static unsigned int gate_valve_D03_status = 385U;
	static unsigned int gate_valve_D04_status = 377U;
	static unsigned int gate_valve_D05_status = 401U;
	static unsigned int gate_valve_D06_status = 409U;
	static unsigned int gate_valve_D07_status = 417U;
	static unsigned int gate_valve_D08_status = 425U;
	static unsigned int gate_valve_D09_status = 433U;
	static unsigned int gate_valve_D10_status = 441U;
	static unsigned int gate_valve_D11_status = 449U;
	static unsigned int gate_valve_D12_status = 457U;
	static unsigned int gate_valve_D13_status = 465U;
	static unsigned int gate_valve_D14_status = 473U;
	static unsigned int gate_valve_D15_status = 481U;
	static unsigned int gate_valve_D16_status = 489U;
	static unsigned int gate_valve_D17_status = 497U;
	static unsigned int gate_valve_D18_status = 505U;
	static unsigned int gate_valve_D19_status = 513U;
	static unsigned int gate_valve_D20_status = 521U;
	static unsigned int gate_valve_D21_status = 537U;
	static unsigned int gate_valve_D22_status = 529U;
	static unsigned int gate_valve_D23_status = 545U;
	static unsigned int gate_valve_D24_status = 553U;
	static unsigned int gate_valve_D25_status = 561U;
	static unsigned int gate_valve_D26_status = 569U;

	static unsigned int motor_valve_D01_status = 2817U;
	static unsigned int motor_valve_D02_status = 2841U;
	static unsigned int motor_valve_D03_status = 2833U;
	static unsigned int motor_valve_D04_status = 2825U;
	static unsigned int motor_valve_D05_status = 2849U;
	static unsigned int motor_valve_D06_status = 2857U;
	static unsigned int motor_valve_D07_status = 2865U;
	static unsigned int motor_valve_D08_status = 2873U;
	static unsigned int motor_valve_D09_status = 2881U;
	static unsigned int motor_valve_D10_status = 2889U;
	static unsigned int motor_valve_D11_status = 2897U;
	static unsigned int motor_valve_D12_status = 2905U;
	static unsigned int motor_valve_D13_status = 2913U;
	static unsigned int motor_valve_D14_status = 2921U;
	static unsigned int motor_valve_D15_status = 2929U;
	static unsigned int motor_valve_D16_status = 2937U;
	static unsigned int motor_valve_D17_status = 2945U;
	static unsigned int motor_valve_D18_status = 2953U;
	static unsigned int motor_valve_D19_status = 2961U;
	static unsigned int motor_valve_D20_status = 2969U;
	static unsigned int motor_valve_D21_status = 2985U;
	static unsigned int motor_valve_D22_status = 2977U;
	static unsigned int motor_valve_D23_status = 2993U;
	static unsigned int motor_valve_D24_status = 3001U;
	static unsigned int motor_valve_D25_status = 3009U;
	static unsigned int motor_valve_D26_status = 3017U;

	static unsigned int butterfly_valve_HBV01_status = 161U;
	static unsigned int butterfly_valve_HBV02_status = 169U;
	static unsigned int butterfly_valve_HBV03_status = 177U;
	static unsigned int butterfly_valve_HBV04_status = 185U;
	static unsigned int butterfly_valve_HBV05_status = 193U;
	static unsigned int butterfly_valve_HBV06_status = 201U;
	static unsigned int butterfly_valve_HBV07_status = 209U;
	static unsigned int butterfly_valve_HBV08_status = 217U;
	static unsigned int butterfly_valve_HBV09_status = 225U;
	static unsigned int butterfly_valve_HBV10_status = 233U;
	static unsigned int butterfly_valve_HBV11_status = 241U;
	static unsigned int butterfly_valve_HBV12_status = 249U;
	static unsigned int butterfly_valve_HBV13_status = 257U;
	static unsigned int butterfly_valve_HBV14_status = 265U;
	static unsigned int butterfly_valve_HBV15_status = 273U;
	static unsigned int butterfly_valve_HBV16_status = 281U;
	static unsigned int butterfly_valve_HBV17_status = 289U;
	static unsigned int butterfly_valve_HBV18_status = 297U;

	/*********************************************************************************************/
	void DI_manual_valve(WarGrey::SCADA::ManualValvelet* target, const uint8* db4, size_t idx_p1);
	void DI_gate_valve(WarGrey::SCADA::GateValvelet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);
	void DI_motor_valve(WarGrey::SCADA::MotorValvelet* target, const uint8* db4, size_t idx4_p1, const uint8* db205, size_t idx205_p1);

	bool DI_manual_valve_open(const uint8* db4, size_t idx_p1);
}
