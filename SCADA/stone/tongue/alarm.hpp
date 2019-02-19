#pragma once

#include "tongue.hpp"

namespace WarGrey::SCADA {
    private class Alarms : public WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms> {
        friend class WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>;
    public:
        static Platform::String^ type() { return "alarm"; }

    public:
        static WarGrey::SCADA::Alarms* PSHopperMaintenance() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262151U); } // PS Hopper/Underwater Converter Mainenance
        static WarGrey::SCADA::Alarms* PSGlandPumpABroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262155U); } // PS Master Gland Pump Broken
        static WarGrey::SCADA::Alarms* PSGlandPumpBBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262159U); } // PS Spare Gland Pump Broken
        static WarGrey::SCADA::Alarms* PSWaterPumpAlert() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262163U); } // PS Water Pump Alert
        static WarGrey::SCADA::Alarms* PSWaterPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262164U); } // PS Water Pump Broken
        static WarGrey::SCADA::Alarms* PSWaterPumpMaintenance() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262166U); } // PS Water Pump Maintenance
        static WarGrey::SCADA::Alarms* PSWaterPumpEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262167U); } // PS Water Pump Emergence
        static WarGrey::SCADA::Alarms* SBHopperMaintenance() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262175U); } // SB Hopper/Underwater Converter Mainenance
        static WarGrey::SCADA::Alarms* SBGlandPumpABroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262179U); } // SB Master Gland Pump Broken
        static WarGrey::SCADA::Alarms* SBGlandPumpBBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262183U); } // SB Spare Gland Pump Broken
        static WarGrey::SCADA::Alarms* SBWaterPumpAlert() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262187U); } // SB Water Pump Alert
        static WarGrey::SCADA::Alarms* SBWaterPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262188U); } // SB Water Pump Broken
        static WarGrey::SCADA::Alarms* SBWaterPumpMaintenance() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262191U); } // SB Water Pump Emergence
        static WarGrey::SCADA::Alarms* PSDHPumpABroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262194U); } // PS Drag Head Pump A Broken
        static WarGrey::SCADA::Alarms* PSDIPumpBBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262198U); } // PS Drag Intermediate Pump B Broken
        static WarGrey::SCADA::Alarms* PSDTPumpCBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262202U); } // PS Drag Trunnion Pump C Broken
        static WarGrey::SCADA::Alarms* SBDVPumpIBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262206U); } // SB Drag Visor Pump I Broken
        static WarGrey::SCADA::Alarms* SBDHPumpHBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262210U); } // SB Drag Head Pump H Broken
        static WarGrey::SCADA::Alarms* SBDIPumpGBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262214U); } // SB Drag Intermediate Pump G Broken
        static WarGrey::SCADA::Alarms* SBDTPumpFBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262218U); } // SB Drag Trunnion Pump F Broken
        static WarGrey::SCADA::Alarms* SBDVPumpJBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262222U); } // SB Drag Visor Pump J Broken
        static WarGrey::SCADA::Alarms* DLBVPumpDBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262226U); } // Door Locker/Butterfly Valve Pump D Broken
        static WarGrey::SCADA::Alarms* DLBVPumpEBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262230U); } // Door Locker/Butterfly Valve Pump E Broken
        static WarGrey::SCADA::Alarms* CoolanPumpKBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262234U); } // Coolant Pump K Broken
        static WarGrey::SCADA::Alarms* MotorFlushingPumpLBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262238U); } // Motor Flushing Pump L Broken
        static WarGrey::SCADA::Alarms* SparePumpMBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262242U); } // Spare Pump M Broken
        static WarGrey::SCADA::Alarms* EmergencePumpYBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262246U); } // Emergence Pump Y Broken
        static WarGrey::SCADA::Alarms* PSGVFlushingPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262250U); } // PS Gate Valve Flushing Pump Broken
        static WarGrey::SCADA::Alarms* SBGVFlushingPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262254U); } // SB Gate Valve Flushing Pump Broken
        static WarGrey::SCADA::Alarms* MasterTankLS2() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262262U); } // Master Tank Level Ultra Low
        static WarGrey::SCADA::Alarms* VisorTankLS2() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262274U); } // Drag Visor Tank Level Ultra Low
        static WarGrey::SCADA::Alarms* PumpA2C() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262288U); } // Pump A Replace C
        static WarGrey::SCADA::Alarms* PumpC2A() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262289U); } // Pump C Replace A
        static WarGrey::SCADA::Alarms* PumpB2C() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262290U); } // Pump B Replace C
        static WarGrey::SCADA::Alarms* PumpC2B() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262291U); } // Pump C Replace B
        static WarGrey::SCADA::Alarms* PumpF2C() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262292U); } // Pump F Replace C
        static WarGrey::SCADA::Alarms* PumpC2F() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262293U); } // Pump C Replace F
        static WarGrey::SCADA::Alarms* PumpH2F() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262294U); } // Pump H Replace F
        static WarGrey::SCADA::Alarms* PumpF2H() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262295U); } // Pump F Replace H
        static WarGrey::SCADA::Alarms* PumpG2F() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262296U); } // Pump G Replace F
        static WarGrey::SCADA::Alarms* PumpF2G() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262297U); } // Pump F Replace G
        static WarGrey::SCADA::Alarms* PumpI2J() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262298U); } // Pump I Replace J
        static WarGrey::SCADA::Alarms* PumpJ2I() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262299U); } // Pump J Replace I
        static WarGrey::SCADA::Alarms* ShoreWinchEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262307U); } // Shore Dischange Winch Emergence
        static WarGrey::SCADA::Alarms* PSDTEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262319U); } // PS Drag Trunnion Emergence
        static WarGrey::SCADA::Alarms* PSDIEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262327U); } // PS Drag Intermediate Emergence
        static WarGrey::SCADA::Alarms* PSDHEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262335U); } // PS Drag Head Emergence
        static WarGrey::SCADA::Alarms* SBDTEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262343U); } // SB Drag Trunnion Emergence
        static WarGrey::SCADA::Alarms* SBDIEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262351U); } // SB Drag Intermediate Emergence
        static WarGrey::SCADA::Alarms* SBDHEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262359U); } // SB Drag Head Emergence
        static WarGrey::SCADA::Alarms* BargeWinchEmergence() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262367U); } // Barge Winch Emergence
        static WarGrey::SCADA::Alarms* PSHopperUnitPressureLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262629U); } // PS Hopper Lubricating Unit Pressure Low
        static WarGrey::SCADA::Alarms* PSHopperUnitLevelLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262630U); } // PS Hopper Lubricating Unit Level Low
        static WarGrey::SCADA::Alarms* PSHopperUnitOilTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262631U); } // PS Hopper Lubricating Unit Oil Temperature High
        static WarGrey::SCADA::Alarms* PSHopperUnitWaterTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262632U); } // PS Hopper Lubricating Unit Water Temperature High
        static WarGrey::SCADA::Alarms* PSHopperUnitBearingTemperatureHigh1() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262633U); } // PS Hopper Lubricating Unit Bearing Temperature 1# High
        static WarGrey::SCADA::Alarms* PSHopperUnitBearingTemperatureHigh2() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262634U); } // PS Hopper Lubricating Unit Bearing Temperature 2# High
        static WarGrey::SCADA::Alarms* PSHopperUnitBearingTemperatureHigh3() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262635U); } // PS Hopper Lubricating Unit Bearing Temperature 3# High
        static WarGrey::SCADA::Alarms* SBHopperUnitPressureLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262645U); } // SB Hopper Lubricating Unit Pressure Low
        static WarGrey::SCADA::Alarms* SBHopperUnitLevelLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262646U); } // SB Hopper Lubricating Unit Level Low
        static WarGrey::SCADA::Alarms* SBHopperUnitOilTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262647U); } // SB Hopper Lubricating Unit Oil Temperature High
        static WarGrey::SCADA::Alarms* SBHopperUnitWaterTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262648U); } // SB Hopper Lubricating Unit Water Temperature High
        static WarGrey::SCADA::Alarms* SBHopperUnitBearingTemperatureHigh1() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262649U); } // SB Hopper Lubricating Unit Bearing Temperature 1# High
        static WarGrey::SCADA::Alarms* SBHopperUnitBearingTemperatureHigh2() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262650U); } // SB Hopper Lubricating Unit Bearing Temperature 2# High
        static WarGrey::SCADA::Alarms* SBHopperUnitBearingTemperatureHigh3() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262651U); } // SB Hopper Lubricating Unit Bearing Temperature 3# High
        static WarGrey::SCADA::Alarms* PSUnderWaterGlandPump1Broken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262659U); } // PS Underwater Master Gland Pump 1# Broken
        static WarGrey::SCADA::Alarms* PSUnderWaterGlandPump2Broken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262663U); } // PS Underwater Master Gland Pump 2# Broken
        static WarGrey::SCADA::Alarms* PSHopperGearboxMasterPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262666U); } // PS Hopper Gearbox Master Pump Broken
        static WarGrey::SCADA::Alarms* PSHopperGearboxSparePumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262669U); } // PS Hopper Gearbox Spare Pump Broken
        static WarGrey::SCADA::Alarms* PSHopperGearboxTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262670U); } // PS Hopper Gearbox Temperature High
        static WarGrey::SCADA::Alarms* PSHopperGearboxPressureLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262671U); } // PS Hopper Gearbox Pressure Low
        static WarGrey::SCADA::Alarms* SBUnderWaterGlandPump1Broken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262675U); } // SB Underwater Master Gland Pump 1# Broken
        static WarGrey::SCADA::Alarms* SBUnderWaterGlandPump2Broken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262679U); } // SB Underwater Master Gland Pump 2# Broken
        static WarGrey::SCADA::Alarms* SBHopperGearboxMasterPumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262682U); } // SB Hopper Gearbox Master Pump Broken
        static WarGrey::SCADA::Alarms* SBHopperGearboxSparePumpBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262685U); } // SB Hopper Gearbox Spare Pump Broken
        static WarGrey::SCADA::Alarms* SBHopperGearboxTemperatureHigh() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262686U); } // SB Hopper Gearbox Temperature High
        static WarGrey::SCADA::Alarms* SBHopperGearboxPressureLow() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262687U); } // SB Hopper Gearbox Pressure Low
        static WarGrey::SCADA::Alarms* HydraulicsStoppedBySailing() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(262812U); } // Hydraulics Stopped(Sailing Console)
        static WarGrey::SCADA::Alarms* PSDIAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436298U); } // PS Drag Intermediate Angle Exceed
        static WarGrey::SCADA::Alarms* SBDIAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436299U); } // SB Drag Intermediate Angle Exceed
        static WarGrey::SCADA::Alarms* BackOilPressureFail() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436689U); } // Back Oil Pressure Less Than 3bar, All Winches are Limited
        static WarGrey::SCADA::Alarms* ET200M_BowPLC1Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436936U); } // 11#ET200M - Bow PLC 1 Lost
        static WarGrey::SCADA::Alarms* ET200M_BowPLC2Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436937U); } // 12#ET200M - Bow PLC 2 ET200M Lost
        static WarGrey::SCADA::Alarms* ET200M_BowPLC3Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436938U); } // 13#ET200M - Bow PLC 3 ET200M Lost
        static WarGrey::SCADA::Alarms* ET200M_BowPLC4Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436939U); } // 14#ET200M - Bow PLC 4 ET200M Lost
        static WarGrey::SCADA::Alarms* ET200M_BowPLC5Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436940U); } // 15#ET200M - Bow PLC 5 ET200M Lost
        static WarGrey::SCADA::Alarms* ET200M_ConsoleDCC1Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436941U); } // 16#ET200M - Dredging Console DCC1 Lost
        static WarGrey::SCADA::Alarms* ET200M_ConsoleDCC2Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436942U); } // 17#ET200M - Dredging Console DCC2 Lost
        static WarGrey::SCADA::Alarms* PSDTWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436943U); } // 21# - PS Drag Trunnion Winch Lost
        static WarGrey::SCADA::Alarms* SBDTWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436944U); } // 22# - SB Drag Trunnion Winch Lost
        static WarGrey::SCADA::Alarms* PSDIWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436945U); } // 23# - PS Drag Intermediate Winch Lost
        static WarGrey::SCADA::Alarms* SBDIWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436946U); } // 24# - SB Drag Intermediate Winch Lost
        static WarGrey::SCADA::Alarms* PSDHWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436947U); } // 25# - PS Drag Head Winch Lost
        static WarGrey::SCADA::Alarms* SBDHWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436948U); } // 26# - SB Drag Head Winch Lost
        static WarGrey::SCADA::Alarms* BargeWinchLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436949U); } // 27# - Barge Winch Lost
        static WarGrey::SCADA::Alarms* MasterCPU_BowPLC1Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436950U); } // 2#CPU(Master) - Bow PLC 1 Lost
        static WarGrey::SCADA::Alarms* SpareCPU_BowPLC1Lost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436951U); } // 3#CPU(Spare) - Bow PLC 1 Lost
        static WarGrey::SCADA::Alarms* DCSDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436952U); } // 18# DCS DP Lost
        static WarGrey::SCADA::Alarms* PSHPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436953U); } // 31# PS Hopper Pump DP Lost
        static WarGrey::SCADA::Alarms* SBHPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436954U); } // 32# SB Hopper Pump DP Lost
        static WarGrey::SCADA::Alarms* PSWPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436955U); } // 33# PS Water Pump DP Lost
        static WarGrey::SCADA::Alarms* SBWPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436956U); } // 34# SB Water Pump DP Lost
        static WarGrey::SCADA::Alarms* PSUWPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436957U); } // 35# PS Underwater Pump DP Lost
        static WarGrey::SCADA::Alarms* SBUWPDPLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436958U); } // 36# PS Underwater Pump DP Lost
        static WarGrey::SCADA::Alarms* PLCPowerBroken() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436959U); } // PLC Power Broken
        static WarGrey::SCADA::Alarms* TildemeterLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436961U); } // Tildemeter Lost
        static WarGrey::SCADA::Alarms* GPSLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436962U); } // GPS Lost
        static WarGrey::SCADA::Alarms* GyrocompassLost() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13436963U); } // Gyrocompass Lost
        static WarGrey::SCADA::Alarms* PSFAAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437168U); } // PS Forearm Drag Angle Exceed
        static WarGrey::SCADA::Alarms* PSBAAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437169U); } // PS Backarm Drag Angle Exceed
        static WarGrey::SCADA::Alarms* PSFBAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437170U); } // PS Fore-Back Angle Exceed
        static WarGrey::SCADA::Alarms* PSDTCableExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437171U); } // PS Drag Trunnion Cable Exceed
        static WarGrey::SCADA::Alarms* SBFAAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437172U); } // SB Forearm Drag Angle Exceed
        static WarGrey::SCADA::Alarms* SBBAAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437173U); } // SB Backarm Drag Angle Exceed
        static WarGrey::SCADA::Alarms* SBFBAngleExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437174U); } // SB Fore-Back Angle Exceed
        static WarGrey::SCADA::Alarms* SBDTCableExceed() { return WarGrey::SCADA::Tongue<WarGrey::SCADA::Alarms>::UnsafeTongue(13437175U); } // SB Drag Trunnion Cable Exceed

    private:
        static size_t indices_size() { return 116U; }
        static const unsigned int* indices() {
            static const unsigned int indexes[] = {
                262151U, 262155U, 262159U, 262163U, 262164U, 262166U, 262167U, 262175U,
                262179U, 262183U, 262187U, 262188U, 262191U, 262194U, 262198U, 262202U,
                262206U, 262210U, 262214U, 262218U, 262222U, 262226U, 262230U, 262234U,
                262238U, 262242U, 262246U, 262250U, 262254U, 262262U, 262274U, 262288U,
                262289U, 262290U, 262291U, 262292U, 262293U, 262294U, 262295U, 262296U,
                262297U, 262298U, 262299U, 262307U, 262319U, 262327U, 262335U, 262343U,
                262351U, 262359U, 262367U, 262629U, 262630U, 262631U, 262632U, 262633U,
                262634U, 262635U, 262645U, 262646U, 262647U, 262648U, 262649U, 262650U,
                262651U, 262659U, 262663U, 262666U, 262669U, 262670U, 262671U, 262675U,
                262679U, 262682U, 262685U, 262686U, 262687U, 262812U, 13436298U, 13436299U,
                13436689U, 13436936U, 13436937U, 13436938U, 13436939U, 13436940U, 13436941U, 13436942U,
                13436943U, 13436944U, 13436945U, 13436946U, 13436947U, 13436948U, 13436949U, 13436950U,
                13436951U, 13436952U, 13436953U, 13436954U, 13436955U, 13436956U, 13436957U, 13436958U,
                13436959U, 13436961U, 13436962U, 13436963U, 13437168U, 13437169U, 13437170U, 13437171U,
                13437172U, 13437173U, 13437174U, 13437175U
            };

            return indexes;
        }

    private:
        Alarms(unsigned int idx) : Tongue(idx) {}
    };
}

