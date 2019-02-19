#lang racket

(provide (all-defined-out)
         (rename-out [main alarm-tongues]))

(require syntax/location)

(require "../../../Scripts/catalogue/tongue.rkt")

(define line-splite
  (lambda [line]
    (define tokens (regexp-match #px"DB(\\d+)[.]DBX(\\d+)[.]([0-7]);[^/]+//\\s*(.+)\\s*$" line))
    (and (pair? tokens)
         (list (string->number (cadr tokens))
               (string->number (caddr tokens))
               (string->number (cadddr tokens))
               (last tokens)))))

(define alarm-tongue-index
  (lambda [DB idx bidx]
    (+ (arithmetic-shift DB 16)
       (+ (* idx 8) bidx))))

(define alarm-db-index
  (lambda [tongue-index]
    (values (arithmetic-shift tongue-index -16)
            (bitwise-and tongue-index #xFFFF))))

(define identify
  (lambda [tokens]
    (match-define (list DB idx bidx zh_CN) tokens)
    (define e
      (case zh_CN
        [("左舷舱内泵/水下泵变频器维修模式") (cons 'PSHopperMaintenance "PS Hopper/Underwater Converter Mainenance")] ;;; 39[DB4.DBX0.7]
        [("左舷舱内泵封水泵变频器A故障状态") (cons 'PSGlandPumpABroken "PS Master Gland Pump Broken")] ;;; 43[DB4.DBX1.3]
        [("左舷舱内泵封水泵变频器B故障状态") (cons 'PSGlandPumpBBroken "PS Spare Gland Pump Broken")] ;;; 47[DB4.DBX1.7]
        [("左舷高压冲水泵变频器报警状态") (cons 'PSWaterPumpAlert "PS Water Pump Alert")] ;;; 51[DB4.DBX2.3]
        [("左舷高压冲水泵变频器故障状态") (cons 'PSWaterPumpBroken "PS Water Pump Broken")] ;;; 52[DB4.DBX2.4]
        [("左舷高压冲水泵变频器维修模式") (cons 'PSWaterPumpMaintenance "PS Water Pump Maintenance")] ;;; 54[DB4.DBX2.6]
        [("左舷高压冲水泵变频器应急停止反馈") (cons 'PSWaterPumpEmergence "PS Water Pump Emergence")] ;;; 55[DB4.DBX2.7]
        [("右舷舱内泵/水下泵变频器维修模式") (cons 'SBHopperMaintenance "SB Hopper/Underwater Converter Mainenance")] ;;; 63[DB4.DBX3.7]
        [("右舷舱内泵封水泵变频器A故障状态") (cons 'SBGlandPumpABroken "SB Master Gland Pump Broken")] ;;; 67[DB4.DBX4.3]
        [("右舷舱内泵封水泵变频器B故障状态") (cons 'SBGlandPumpBBroken "SB Spare Gland Pump Broken")] ;;; 71[DB4.DBX4.7]
        [("右舷高压冲水泵变频器报警状态") (cons 'SBWaterPumpAlert "SB Water Pump Alert")] ;;; 75[DB4.DBX5.3]
        [("右舷高压冲水泵变频器故障状态") (cons 'SBWaterPumpBroken "SB Water Pump Broken")] ;;; 76[DB4.DBX5.4]
        [("右舷高压冲水泵变频器应急停止反馈") (cons 'SBWaterPumpMaintenance "SB Water Pump Emergence")] ;;; 79[DB4.DBX5.7]
        [("左舷耙头液压泵A故障反馈") (cons 'PSDHPumpABroken "PS Drag Head Pump A Broken")] ;;; 82[DB4.DBX6.2]
        [("左舷耙中液压泵B故障反馈") (cons 'PSDIPumpBBroken "PS Drag Intermediate Pump B Broken")] ;;; 86[DB4.DBX6.6]
        [("左舷弯管液压泵C故障反馈") (cons 'PSDTPumpCBroken "PS Drag Trunnion Pump C Broken")] ;;; 90[DB4.DBX7.2]
        [("右舷耙唇液压泵I故障反馈") (cons 'SBDVPumpIBroken "SB Drag Visor Pump I Broken")] ;;; 94[DB4.DBX7.6]
        [("右舷耙头液压泵H故障反馈") (cons 'SBDHPumpHBroken "SB Drag Head Pump H Broken")] ;;; 98[DB4.DBX8.2]
        [("右舷耙中液压泵G故障反馈") (cons 'SBDIPumpGBroken "SB Drag Intermediate Pump G Broken")] ;;; 102[DB4.DBX8.6]
        [("右舷弯管液压泵F故障反馈") (cons 'SBDTPumpFBroken "SB Drag Trunnion Pump F Broken")] ;;; 106[DB4.DBX9.2]
        [("左舷耙唇液压泵J故障反馈") (cons 'SBDVPumpJBroken "SB Drag Visor Pump J Broken")] ;;; 110[DB4.DBX9.6]
        [("泥泵锁紧/蝶阀控制泵D故障反馈") (cons 'DLBVPumpDBroken "Door Locker/Butterfly Valve Pump D Broken")] ;;; 114[DB4.DBX10.2]
        [("泥泵锁紧/蝶阀控制泵E故障反馈") (cons 'DLBVPumpEBroken "Door Locker/Butterfly Valve Pump E Broken")] ;;; 118[DB4.DBX10.6]
        [("液压冷却泵K故障反馈") (cons 'CoolanPumpKBroken "Coolant Pump K Broken")] ;;; 122[DB4.DBX11.2]
        [("马达冲洗液压泵L故障反馈") (cons 'MotorFlushingPumpLBroken "Motor Flushing Pump L Broken")] ;;; 126[DB4.DBX11.6]
        [("冷却/马达冲洗备用泵M故障反馈") (cons 'SparePumpMBroken "Spare Pump M Broken")] ;;; 130[DB4.DBX12.2]
        [("应急液压泵Y故障反馈") (cons 'EmergencePumpYBroken "Emergence Pump Y Broken")] ;;; 134[DB4.DBX12.6]
        [("左舷闸阀冲洗泵故障反馈") (cons 'PSGVFlushingPumpBroken "PS Gate Valve Flushing Pump Broken")] ;;; 138[DB4.DBX13.2]
        [("右舷闸阀冲洗泵故障反馈") (cons 'SBGVFlushingPumpBroken "SB Gate Valve Flushing Pump Broken")] ;;; 142[DB4.DBX13.6]
        [("液压主系统油箱液位低低LS.2") (cons 'MasterTankLS2 "Master Tank Level Ultra Low")] ;;; 150[DB4.DBX14.6]
        [("耙唇液压系统油箱液位低低LS.12") (cons 'VisorTankLS2 "Drag Visor Tank Level Ultra Low")] ;;; 162[DB4.DBX16.2]
        [("A替代C") (cons 'PumpA2C "Pump A Replace C")] ;;; 176[DB4.DBX18.0]
        [("C替代A") (cons 'PumpC2A "Pump C Replace A")] ;;; 177[DB4.DBX18.1]
        [("B替代C") (cons 'PumpB2C "Pump B Replace C")] ;;; 178[DB4.DBX18.2]
        [("C替代B") (cons 'PumpC2B "Pump C Replace B")] ;;; 179[DB4.DBX18.3]
        [("F替代C") (cons 'PumpF2C "Pump F Replace C")] ;;; 180[DB4.DBX18.4]
        [("C替代F") (cons 'PumpC2F "Pump C Replace F")] ;;; 181[DB4.DBX18.5]
        [("H替代F") (cons 'PumpH2F "Pump H Replace F")] ;;; 182[DB4.DBX18.6]
        [("F替代H") (cons 'PumpF2H "Pump F Replace H")] ;;; 183[DB4.DBX18.7]
        [("G替代F") (cons 'PumpG2F "Pump G Replace F")] ;;; 184[DB4.DBX19.0]
        [("F替代G") (cons 'PumpF2G "Pump F Replace G")] ;;; 185[DB4.DBX19.1]
        [("I替代J") (cons 'PumpI2J "Pump I Replace J")] ;;; 186[DB4.DBX19.2]
        [("J替代I") (cons 'PumpJ2I "Pump J Replace I")] ;;; 187[DB4.DBX19.3]
        [("艏吹绞车紧急停止") (cons 'ShoreWinchEmergence "Shore Dischange Winch Emergence")] ;;; 195[DB4.DBX20.3]
        [("左舷弯管紧急停止") (cons 'PSDTEmergence "PS Drag Trunnion Emergence")] ;;; 207[DB4.DBX21.7]
        [("左舷耙中紧急停止") (cons 'PSDIEmergence "PS Drag Intermediate Emergence")] ;;; 215[DB4.DBX22.7]
        [("左舷耙头紧急停止") (cons 'PSDHEmergence "PS Drag Head Emergence")] ;;; 223[DB4.DBX23.7]
        [("右舷弯管紧急停止") (cons 'SBDTEmergence "SB Drag Trunnion Emergence")] ;;; 231[DB4.DBX24.7]
        [("右舷耙中紧急停止") (cons 'SBDIEmergence "SB Drag Intermediate Emergence")] ;;; 239[DB4.DBX25.7]
        [("右舷耙头紧急停止") (cons 'SBDHEmergence "SB Drag Head Emergence")] ;;; 247[DB4.DBX26.7]
        [("装驳绞车紧急停止") (cons 'BargeWinchEmergence "Barge Winch Emergence")] ;;; 255[DB4.DBX27.7]
        [("左舷泥泵轴承润滑单元压力低报警") (cons 'PSHopperUnitPressureLow "PS Hopper Lubricating Unit Pressure Low")] ;;; 517[DB4.DBX60.5]
        [("左舷泥泵轴承润滑单元液位低报警") (cons 'PSHopperUnitLevelLow "PS Hopper Lubricating Unit Level Low")] ;;; 518[DB4.DBX60.6]
        [("左舷泥泵轴承润滑单元油温高报警") (cons 'PSHopperUnitOilTemperatureHigh "PS Hopper Lubricating Unit Oil Temperature High")] ;;; 519[DB4.DBX60.7]
        [("左舷泥泵轴承润滑单元水温高报警") (cons 'PSHopperUnitWaterTemperatureHigh "PS Hopper Lubricating Unit Water Temperature High")] ;;; 520[DB4.DBX61.0]
        [("左舷泥泵轴承润滑单元轴承温度高1#报警") (cons 'PSHopperUnitBearingTemperatureHigh1 "PS Hopper Lubricating Unit Bearing Temperature 1# High")] ;;; 521[DB4.DBX61.1]
        [("左舷泥泵轴承润滑单元轴承温度高2#报警") (cons 'PSHopperUnitBearingTemperatureHigh2 "PS Hopper Lubricating Unit Bearing Temperature 2# High")] ;;; 522[DB4.DBX61.2]
        [("左舷泥泵轴承润滑单元轴承温度高3#报警") (cons 'PSHopperUnitBearingTemperatureHigh3 "PS Hopper Lubricating Unit Bearing Temperature 3# High")] ;;; 523[DB4.DBX61.3]
        [("右舷泥泵轴承润滑单元压力低报警") (cons 'SBHopperUnitPressureLow "SB Hopper Lubricating Unit Pressure Low")] ;;; 533[DB4.DBX62.5]
        [("右舷泥泵轴承润滑单元液位低报警") (cons 'SBHopperUnitLevelLow "SB Hopper Lubricating Unit Level Low")] ;;; 534[DB4.DBX62.6]
        [("右舷泥泵轴承润滑单元油温高报警") (cons 'SBHopperUnitOilTemperatureHigh "SB Hopper Lubricating Unit Oil Temperature High")] ;;; 535[DB4.DBX62.7]
        [("右舷泥泵轴承润滑单元水温高报警") (cons 'SBHopperUnitWaterTemperatureHigh "SB Hopper Lubricating Unit Water Temperature High")] ;;; 536[DB4.DBX63.0]
        [("右舷泥泵轴承润滑单元轴承温度高1#报警") (cons 'SBHopperUnitBearingTemperatureHigh1 "SB Hopper Lubricating Unit Bearing Temperature 1# High")] ;;; 537[DB4.DBX63.1]
        [("右舷泥泵轴承润滑单元轴承温度高2#报警") (cons 'SBHopperUnitBearingTemperatureHigh2 "SB Hopper Lubricating Unit Bearing Temperature 2# High")] ;;; 538[DB4.DBX63.2]
        [("右舷泥泵轴承润滑单元轴承温度高3#报警") (cons 'SBHopperUnitBearingTemperatureHigh3 "SB Hopper Lubricating Unit Bearing Temperature 3# High")] ;;; 539[DB4.DBX63.3]
        [("左舷水下泵1#封水泵故障反馈") (cons 'PSUnderWaterGlandPump1Broken "PS Underwater Master Gland Pump 1# Broken")] ;;; 547[DB4.DBX64.3]
        [("左舷水下泵2#封水泵故障反馈") (cons 'PSUnderWaterGlandPump2Broken "PS Underwater Master Gland Pump 2# Broken")] ;;; 551[DB4.DBX64.7]
        [("左舷泥泵齿轮箱电动滑油泵故障反馈") (cons 'PSHopperGearboxMasterPumpBroken "PS Hopper Gearbox Master Pump Broken")] ;;; 554[DB4.DBX65.2]
        [("左舷泥泵齿轮箱备用电动滑油泵故障反馈") (cons 'PSHopperGearboxSparePumpBroken "PS Hopper Gearbox Spare Pump Broken")] ;;; 557[DB4.DBX65.5]
        [("左舷泥泵齿轮箱滑油温度高报警") (cons 'PSHopperGearboxTemperatureHigh "PS Hopper Gearbox Temperature High")] ;;; 558[DB4.DBX65.6]
        [("左舷泥泵齿轮箱滑油压力低低报警") (cons 'PSHopperGearboxPressureLow "PS Hopper Gearbox Pressure Low")] ;;; 559[DB4.DBX65.7]
        [("右舷水下泵1#封水泵故障反馈") (cons 'SBUnderWaterGlandPump1Broken "SB Underwater Master Gland Pump 1# Broken")] ;;; 563[DB4.DBX66.3]
        [("右舷水下泵2#封水泵故障反馈") (cons 'SBUnderWaterGlandPump2Broken "SB Underwater Master Gland Pump 2# Broken")] ;;; 567[DB4.DBX66.7]
        [("右舷泥泵齿轮箱电动滑油泵故障反馈") (cons 'SBHopperGearboxMasterPumpBroken "SB Hopper Gearbox Master Pump Broken")] ;;; 570[DB4.DBX67.2]
        [("右舷泥泵齿轮箱备用电动滑油泵故障反馈") (cons 'SBHopperGearboxSparePumpBroken "SB Hopper Gearbox Spare Pump Broken")] ;;; 573[DB4.DBX67.5]
        [("右舷泥泵齿轮箱滑油温度高报警") (cons 'SBHopperGearboxTemperatureHigh "SB Hopper Gearbox Temperature High")] ;;; 574[DB4.DBX67.6]
        [("右舷泥泵齿轮箱滑油压力低低报警") (cons 'SBHopperGearboxPressureLow "SB Hopper Gearbox Pressure Low")] ;;; 575[DB4.DBX67.7]
        [("液压泵紧急停止(航行台)") (cons 'HydraulicsStoppedBySailing "Hydraulics Stopped(Sailing Console)")] ;;; 700[DB4.DBX83.4]
        
        [("左耙中角度过大") (cons 'PSDIAngleExceed "PS Drag Intermediate Angle Exceed")] ;;; 3050[DB205.DBX177.2]
        [("右耙中角度过大") (cons 'SBDIAngleExceed "SB Drag Intermediate Angle Exceed")] ;;; 3051[DB205.DBX177.3]
        [("回油压力小于3bar,所有绞车不能动作") (cons 'BackOilPressureFail "Back Oil Pressure Less Than 3bar, All Winches are Limited")] ;;; 3441[DB205.DBX226.1]
        [("11#站ET200M-艏PLC柜第1屏通讯故障") (cons 'ET200M_BowPLC1Lost "11#ET200M - Bow PLC 1 Lost")] ;;; 3688[DB205.DBX257.0]
        [("12#站ET200M-艏PLC柜第2屏通讯故障") (cons 'ET200M_BowPLC2Lost "12#ET200M - Bow PLC 2 ET200M Lost")] ;;; 3689[DB205.DBX257.1]
        [("13#站ET200M-艏PLC柜第3屏通讯故障") (cons 'ET200M_BowPLC3Lost "13#ET200M - Bow PLC 3 ET200M Lost")] ;;; 3690[DB205.DBX257.2]
        [("14#站ET200M-艏PLC柜第4屏通讯故障") (cons 'ET200M_BowPLC4Lost "14#ET200M - Bow PLC 4 ET200M Lost")] ;;; 3691[DB205.DBX257.3]
        [("15#站ET200M-艏PLC柜第5屏通讯故障") (cons 'ET200M_BowPLC5Lost "15#ET200M - Bow PLC 5 ET200M Lost")] ;;; 3692[DB205.DBX257.4]
        [("16#站ET200M-疏浚台DCC1通讯故障") (cons 'ET200M_ConsoleDCC1Lost "16#ET200M - Dredging Console DCC1 Lost")] ;;; 3693[DB205.DBX257.5]
        [("17#站ET200M-疏浚台DCC2通讯故障") (cons 'ET200M_ConsoleDCC2Lost "17#ET200M - Dredging Console DCC2 Lost")] ;;; 3694[DB205.DBX257.6]
        [("21#站-左弯管绞车编码器通讯故障") (cons 'PSDTWinchLost "21# - PS Drag Trunnion Winch Lost")] ;;; 3695[DB205.DBX257.7]
        [("22#站-右弯管绞车编码器通讯故障") (cons 'SBDTWinchLost "22# - SB Drag Trunnion Winch Lost")] ;;; 3696[DB205.DBX258.0]
        [("23#站-左耙中绞车编码器通讯故障") (cons 'PSDIWinchLost "23# - PS Drag Intermediate Winch Lost")] ;;; 3697[DB205.DBX258.1]
        [("24#站-右耙中绞车编码器通讯故障") (cons 'SBDIWinchLost "24# - SB Drag Intermediate Winch Lost")] ;;; 3698[DB205.DBX258.2]
        [("25#站-左耙头绞车编码器通讯故障") (cons 'PSDHWinchLost "25# - PS Drag Head Winch Lost")] ;;; 3699[DB205.DBX258.3]
        [("26#站-右耙头绞车编码器通讯故障") (cons 'SBDHWinchLost "26# - SB Drag Head Winch Lost")] ;;; 3700[DB205.DBX258.4]
        [("27#站-装驳绞车编码器通讯故障") (cons 'BargeWinchLost "27# - Barge Winch Lost")] ;;; 3701[DB205.DBX258.5]
        [("2#站CPU(主)-艏PLC柜第1屏通讯故障") (cons 'MasterCPU_BowPLC1Lost "2#CPU(Master) - Bow PLC 1 Lost")] ;;; 3702[DB205.DBX258.6]
        [("3#站CPU(备)-艏PLC柜第1屏通讯故障") (cons 'SpareCPU_BowPLC1Lost "3#CPU(Spare) - Bow PLC 1 Lost")] ;;; 3703[DB205.DBX258.7]
        [("18#站-DCS DP通讯故障") (cons 'DCSDPLost "18# DCS DP Lost")] ;;; 3704[DB205.DBX259.0]
        [("31#站-左舱内泵DP通讯故障") (cons 'PSHPDPLost "31# PS Hopper Pump DP Lost")] ;;; 3705[DB205.DBX259.1]
        [("32#站-右舱内泵DP通讯故障") (cons 'SBHPDPLost "32# SB Hopper Pump DP Lost")] ;;; 3706[DB205.DBX259.2]
        [("33#站-左高压冲水泵DP通讯故障") (cons 'PSWPDPLost "33# PS Water Pump DP Lost")] ;;; 3707[DB205.DBX259.3]
        [("34#站-右高压冲水泵DP通讯故障") (cons 'SBWPDPLost "34# SB Water Pump DP Lost")] ;;; 3708[DB205.DBX259.4]
        [("35#站-左水下泵DP通讯故障") (cons 'PSUWPDPLost "35# PS Underwater Pump DP Lost")] ;;; 3709[DB205.DBX259.5]
        [("36#站-右水下泵DP通讯故障") (cons 'SBUWPDPLost "36# PS Underwater Pump DP Lost")] ;;; 3710[DB205.DBX259.6]
        [("PLC电源模块电池故障") (cons 'PLCPowerBroken "PLC Power Broken")] ;;; 3711[DB205.DBX259.7]
        [("潮位仪通讯故障") (cons 'TildemeterLost "Tildemeter Lost")] ;;; 3713[DB205.DBX260.1]
        [("GPS通讯故障") (cons 'GPSLost "GPS Lost")] ;;; 3714[DB205.DBX260.2]
        [("电罗经通讯故障") (cons 'GyrocompassLost "Gyrocompass Lost")] ;;; 3715[DB205.DBX260.3]
        [("左耙上耙管角度过大报警") (cons 'PSFAAngleExceed "PS Forearm Drag Angle Exceed")] ;;; 3920[DB205.DBX286.0]
        [("左耙下耙管角度过大报警") (cons 'PSBAAngleExceed "PS Backarm Drag Angle Exceed")] ;;; 3921[DB205.DBX286.1]
        [("左耙上下耙管夹角角度过大报警") (cons 'PSFBAngleExceed "PS Fore-Back Angle Exceed")] ;;; 3922[DB205.DBX286.2]
        [("左耙弯管钢丝绳下放长度过大报警") (cons 'PSDTCableExceed "PS Drag Trunnion Cable Exceed")] ;;; 3923[DB205.DBX286.3]
        [("右耙上耙管角度过大报警") (cons 'SBFAAngleExceed "SB Forearm Drag Angle Exceed")] ;;; 3924[DB205.DBX286.4]
        [("右耙下耙管角度过大报警") (cons 'SBBAAngleExceed "SB Backarm Drag Angle Exceed")] ;;; 3925[DB205.DBX286.5]
        [("右耙上下耙管夹角角度过大报警") (cons 'SBFBAngleExceed "SB Fore-Back Angle Exceed")] ;;; 3926[DB205.DBX286.6]
        [("右耙弯管钢丝绳下放长度过大报警") (cons 'SBDTCableExceed "SB Drag Trunnion Cable Exceed")] ;;; 3927[DB205.DBX286.7]
        [else #false]))
    
    (let-values ([(code) (alarm-tongue-index DB idx bidx)]
                 [(id en_US) (if (pair? e) (values (car e) (cdr e)) (values 'ID "en_US"))])
      (unless (pair? e)
        (printf "; [(~s) (cons '~s ~s)] ;;; ~a[DB~a.DBX~a.~a]~n"
                zh_CN id en_US code DB idx bidx))
      (and (pair? e) (tongue id code en_US zh_CN)))))

(define main
  (lambda []
    (parameterize ([current-custodian (make-custodian)])
      (dynamic-wind (λ [] (void))
                    (λ [] (let* ([/dev/txtin (open-input-file (build-path (path-only (quote-source-file)) "alarm.txt"))]
                                 [/dev/stdin (reencode-input-port /dev/txtin "GB18030")])
                            (define alarms (filter-map line-splite (port->lines /dev/stdin)))
                            (values 'Alarms (filter-map (λ [line] (identify line)) alarms))))
                    (λ [] (custodian-shutdown-all (current-custodian)))))))

(module+ main (main))
