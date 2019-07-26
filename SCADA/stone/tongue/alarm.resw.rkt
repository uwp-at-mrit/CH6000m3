#lang racket

(provide (all-defined-out)
         (rename-out [main alarm-tongues]))

(require syntax/location)

(require "../../../../Toolbox/catalogue/tongue.rkt")

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
        [("左舷舱内泵/水下泵变频器维修模式") (cons 'ID "en_US")] ;;; 262151[DB4.DBX0.7]
        [("左舷舱内泵封水泵变频器A故障状态") (cons 'ID "en_US")] ;;; 262155[DB4.DBX1.3]
        [("左舷舱内泵封水泵变频器B故障状态") (cons 'ID "en_US")] ;;; 262159[DB4.DBX1.7]
        [("左舷高压冲水泵变频器报警状态") (cons 'ID "en_US")] ;;; 262163[DB4.DBX2.3]
        [("左舷高压冲水泵变频器故障状态") (cons 'ID "en_US")] ;;; 262164[DB4.DBX2.4]
        [("左舷高压冲水泵变频器维修模式") (cons 'ID "en_US")] ;;; 262166[DB4.DBX2.6]
        [("左舷高压冲水泵变频器应急停止反馈") (cons 'ID "en_US")] ;;; 262167[DB4.DBX2.7]
        [("右舷舱内泵/水下泵变频器维修模式") (cons 'ID "en_US")] ;;; 262175[DB4.DBX3.7]
        [("右舷舱内泵封水泵变频器A故障状态") (cons 'ID "en_US")] ;;; 262179[DB4.DBX4.3]
        [("右舷舱内泵封水泵变频器B故障状态") (cons 'ID "en_US")] ;;; 262183[DB4.DBX4.7]
        [("右舷高压冲水泵变频器报警状态") (cons 'ID "en_US")] ;;; 262187[DB4.DBX5.3]
        [("右舷高压冲水泵变频器故障状态") (cons 'ID "en_US")] ;;; 262188[DB4.DBX5.4]
        [("右舷高压冲水泵变频器应急停止反馈") (cons 'ID "en_US")] ;;; 262191[DB4.DBX5.7]
        [("左舷耙头液压泵A故障反馈") (cons 'ID "en_US")] ;;; 262194[DB4.DBX6.2]
        [("左舷耙中液压泵B故障反馈") (cons 'ID "en_US")] ;;; 262198[DB4.DBX6.6]
        [("左舷弯管液压泵C故障反馈") (cons 'ID "en_US")] ;;; 262202[DB4.DBX7.2]
        [("右舷耙唇液压泵I故障反馈") (cons 'ID "en_US")] ;;; 262206[DB4.DBX7.6]
        [("右舷耙头液压泵H故障反馈") (cons 'ID "en_US")] ;;; 262210[DB4.DBX8.2]
        [("右舷耙中液压泵G故障反馈") (cons 'ID "en_US")] ;;; 262214[DB4.DBX8.6]
        [("右舷弯管液压泵F故障反馈") (cons 'ID "en_US")] ;;; 262218[DB4.DBX9.2]
        [("左舷耙唇液压泵J故障反馈") (cons 'ID "en_US")] ;;; 262222[DB4.DBX9.6]
        [("泥泵锁紧/蝶阀控制泵D故障反馈") (cons 'ID "en_US")] ;;; 262226[DB4.DBX10.2]
        [("泥泵锁紧/蝶阀控制泵E故障反馈") (cons 'ID "en_US")] ;;; 262230[DB4.DBX10.6]
        [("液压冷却泵K故障反馈") (cons 'ID "en_US")] ;;; 262234[DB4.DBX11.2]
        [("马达冲洗液压泵L故障反馈") (cons 'ID "en_US")] ;;; 262238[DB4.DBX11.6]
        [("冷却/马达冲洗备用泵M故障反馈") (cons 'ID "en_US")] ;;; 262242[DB4.DBX12.2]
        [("应急液压泵Y故障反馈") (cons 'ID "en_US")] ;;; 262246[DB4.DBX12.6]
        [("左舷闸阀冲洗泵故障反馈") (cons 'ID "en_US")] ;;; 262250[DB4.DBX13.2]
        [("右舷闸阀冲洗泵故障反馈") (cons 'ID "en_US")] ;;; 262254[DB4.DBX13.6]
        [("液压主系统油箱液位低低LS.2") (cons 'ID "en_US")] ;;; 262262[DB4.DBX14.6]
        [("耙唇液压系统油箱液位低低LS.12") (cons 'ID "en_US")] ;;; 262274[DB4.DBX16.2]
        [("A替代C") (cons 'ID "en_US")] ;;; 262288[DB4.DBX18.0]
        [("C替代A") (cons 'ID "en_US")] ;;; 262289[DB4.DBX18.1]
        [("B替代C") (cons 'ID "en_US")] ;;; 262290[DB4.DBX18.2]
        [("C替代B") (cons 'ID "en_US")] ;;; 262291[DB4.DBX18.3]
        [("F替代C") (cons 'ID "en_US")] ;;; 262292[DB4.DBX18.4]
        [("C替代F") (cons 'ID "en_US")] ;;; 262293[DB4.DBX18.5]
        [("H替代F") (cons 'ID "en_US")] ;;; 262294[DB4.DBX18.6]
        [("F替代H") (cons 'ID "en_US")] ;;; 262295[DB4.DBX18.7]
        [("G替代F") (cons 'ID "en_US")] ;;; 262296[DB4.DBX19.0]
        [("F替代G") (cons 'ID "en_US")] ;;; 262297[DB4.DBX19.1]
        [("I替代J") (cons 'ID "en_US")] ;;; 262298[DB4.DBX19.2]
        [("J替代I") (cons 'ID "en_US")] ;;; 262299[DB4.DBX19.3]
        [("艏吹绞车紧急停止") (cons 'ID "en_US")] ;;; 262307[DB4.DBX20.3]
        [("左舷弯管紧急停止") (cons 'ID "en_US")] ;;; 262319[DB4.DBX21.7]
        [("左舷耙中紧急停止") (cons 'ID "en_US")] ;;; 262327[DB4.DBX22.7]
        [("左舷耙头紧急停止") (cons 'ID "en_US")] ;;; 262335[DB4.DBX23.7]
        [("右舷弯管紧急停止") (cons 'ID "en_US")] ;;; 262343[DB4.DBX24.7]
        [("右舷耙中紧急停止") (cons 'ID "en_US")] ;;; 262351[DB4.DBX25.7]
        [("右舷耙头紧急停止") (cons 'ID "en_US")] ;;; 262359[DB4.DBX26.7]
        [("装驳绞车紧急停止") (cons 'ID "en_US")] ;;; 262367[DB4.DBX27.7]
        [("左舷泥泵轴承润滑单元压力低报警") (cons 'ID "en_US")] ;;; 262629[DB4.DBX60.5]
        [("左舷泥泵轴承润滑单元液位低报警") (cons 'ID "en_US")] ;;; 262630[DB4.DBX60.6]
        [("左舷泥泵轴承润滑单元油温高报警") (cons 'ID "en_US")] ;;; 262631[DB4.DBX60.7]
        [("左舷泥泵轴承润滑单元水温高报警") (cons 'ID "en_US")] ;;; 262632[DB4.DBX61.0]
        [("左舷泥泵轴承润滑单元轴承温度高1#报警") (cons 'ID "en_US")] ;;; 262633[DB4.DBX61.1]
        [("左舷泥泵轴承润滑单元轴承温度高2#报警") (cons 'ID "en_US")] ;;; 262634[DB4.DBX61.2]
        [("左舷泥泵轴承润滑单元轴承温度高3#报警") (cons 'ID "en_US")] ;;; 262635[DB4.DBX61.3]
        [("右舷泥泵轴承润滑单元压力低报警") (cons 'ID "en_US")] ;;; 262645[DB4.DBX62.5]
        [("右舷泥泵轴承润滑单元液位低报警") (cons 'ID "en_US")] ;;; 262646[DB4.DBX62.6]
        [("右舷泥泵轴承润滑单元油温高报警") (cons 'ID "en_US")] ;;; 262647[DB4.DBX62.7]
        [("右舷泥泵轴承润滑单元水温高报警") (cons 'ID "en_US")] ;;; 262648[DB4.DBX63.0]
        [("右舷泥泵轴承润滑单元轴承温度高1#报警") (cons 'ID "en_US")] ;;; 262649[DB4.DBX63.1]
        [("右舷泥泵轴承润滑单元轴承温度高2#报警") (cons 'ID "en_US")] ;;; 262650[DB4.DBX63.2]
        [("右舷泥泵轴承润滑单元轴承温度高3#报警") (cons 'ID "en_US")] ;;; 262651[DB4.DBX63.3]
        [("左舷水下泵1#封水泵故障反馈") (cons 'ID "en_US")] ;;; 262659[DB4.DBX64.3]
        [("左舷水下泵2#封水泵故障反馈") (cons 'ID "en_US")] ;;; 262663[DB4.DBX64.7]
        [("左舷泥泵齿轮箱电动滑油泵故障反馈") (cons 'ID "en_US")] ;;; 262666[DB4.DBX65.2]
        [("左舷泥泵齿轮箱备用电动滑油泵故障反馈") (cons 'ID "en_US")] ;;; 262669[DB4.DBX65.5]
        [("左舷泥泵齿轮箱滑油温度高报警") (cons 'ID "en_US")] ;;; 262670[DB4.DBX65.6]
        [("左舷泥泵齿轮箱滑油压力低低报警") (cons 'ID "en_US")] ;;; 262671[DB4.DBX65.7]
        [("右舷水下泵1#封水泵故障反馈") (cons 'ID "en_US")] ;;; 262675[DB4.DBX66.3]
        [("右舷水下泵2#封水泵故障反馈") (cons 'ID "en_US")] ;;; 262679[DB4.DBX66.7]
        [("右舷泥泵齿轮箱电动滑油泵故障反馈") (cons 'ID "en_US")] ;;; 262682[DB4.DBX67.2]
        [("右舷泥泵齿轮箱备用电动滑油泵故障反馈") (cons 'ID "en_US")] ;;; 262685[DB4.DBX67.5]
        [("右舷泥泵齿轮箱滑油温度高报警") (cons 'ID "en_US")] ;;; 262686[DB4.DBX67.6]
        [("右舷泥泵齿轮箱滑油压力低低报警") (cons 'ID "en_US")] ;;; 262687[DB4.DBX67.7]
        [("液压泵紧急停止(航行台)") (cons 'ID "en_US")] ;;; 262812[DB4.DBX83.4]
        
        [("左高压冲水泵出口阀未打开运行超时报警") (cons 'ID "en_US")] ;;; 13435914[DB205.DBX129.2]
        [("右高压冲水泵出口阀未打开运行超时报警") (cons 'ID "en_US")] ;;; 13435915[DB205.DBX129.3]
        [("左耙中角度过大") (cons 'ID "en_US")] ;;; 13436298[DB205.DBX177.2]
        [("右耙中角度过大") (cons 'ID "en_US")] ;;; 13436299[DB205.DBX177.3]
        [("回油压力小于3bar,所有绞车不能动作") (cons 'ID "en_US")] ;;; 13436689[DB205.DBX226.1]
        [("11#站ET200M-艏PLC柜第1屏通讯故障") (cons 'ID "en_US")] ;;; 13436936[DB205.DBX257.0]
        [("12#站ET200M-艏PLC柜第2屏通讯故障") (cons 'ID "en_US")] ;;; 13436937[DB205.DBX257.1]
        [("13#站ET200M-艏PLC柜第3屏通讯故障") (cons 'ID "en_US")] ;;; 13436938[DB205.DBX257.2]
        [("14#站ET200M-艏PLC柜第4屏通讯故障") (cons 'ID "en_US")] ;;; 13436939[DB205.DBX257.3]
        [("15#站ET200M-艏PLC柜第5屏通讯故障") (cons 'ID "en_US")] ;;; 13436940[DB205.DBX257.4]
        [("16#站ET200M-疏浚台DCC1通讯故障") (cons 'ID "en_US")] ;;; 13436941[DB205.DBX257.5]
        [("17#站ET200M-疏浚台DCC2通讯故障") (cons 'ID "en_US")] ;;; 13436942[DB205.DBX257.6]
        [("21#站-左弯管绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436943[DB205.DBX257.7]
        [("22#站-右弯管绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436944[DB205.DBX258.0]
        [("23#站-左耙中绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436945[DB205.DBX258.1]
        [("24#站-右耙中绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436946[DB205.DBX258.2]
        [("25#站-左耙头绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436947[DB205.DBX258.3]
        [("26#站-右耙头绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436948[DB205.DBX258.4]
        [("27#站-装驳绞车编码器通讯故障") (cons 'ID "en_US")] ;;; 13436949[DB205.DBX258.5]
        [("2#站CPU(主)-艏PLC柜第1屏通讯故障") (cons 'ID "en_US")] ;;; 13436950[DB205.DBX258.6]
        [("3#站CPU(备)-艏PLC柜第1屏通讯故障") (cons 'ID "en_US")] ;;; 13436951[DB205.DBX258.7]
        [("18#站-DCS DP通讯故障") (cons 'ID "en_US")] ;;; 13436952[DB205.DBX259.0]
        [("31#站-左舱内泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436953[DB205.DBX259.1]
        [("32#站-右舱内泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436954[DB205.DBX259.2]
        [("33#站-左高压冲水泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436955[DB205.DBX259.3]
        [("34#站-右高压冲水泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436956[DB205.DBX259.4]
        [("35#站-左水下泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436957[DB205.DBX259.5]
        [("36#站-右水下泵DP通讯故障") (cons 'ID "en_US")] ;;; 13436958[DB205.DBX259.6]
        [("PLC电源模块电池故障") (cons 'ID "en_US")] ;;; 13436959[DB205.DBX259.7]
        [("潮位仪通讯故障") (cons 'ID "en_US")] ;;; 13436961[DB205.DBX260.1]
        [("GPS通讯故障") (cons 'ID "en_US")] ;;; 13436962[DB205.DBX260.2]
        [("电罗经通讯故障") (cons 'ID "en_US")] ;;; 13436963[DB205.DBX260.3]
        [("左耙上耙管角度过大报警") (cons 'ID "en_US")] ;;; 13437168[DB205.DBX286.0]
        [("左耙下耙管角度过大报警") (cons 'ID "en_US")] ;;; 13437169[DB205.DBX286.1]
        [("左耙上下耙管夹角角度过大报警") (cons 'ID "en_US")] ;;; 13437170[DB205.DBX286.2]
        [("左耙弯管钢丝绳下放长度过大报警") (cons 'ID "en_US")] ;;; 13437171[DB205.DBX286.3]
        [("右耙上耙管角度过大报警") (cons 'ID "en_US")] ;;; 13437172[DB205.DBX286.4]
        [("右耙下耙管角度过大报警") (cons 'ID "en_US")] ;;; 13437173[DB205.DBX286.5]
        [("右耙上下耙管夹角角度过大报警") (cons 'ID "en_US")] ;;; 13437174[DB205.DBX286.6]
        [("右耙弯管钢丝绳下放长度过大报警") (cons 'ID "en_US")] ;;; 13437175[DB205.DBX286.7]

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
