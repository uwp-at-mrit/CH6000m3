#lang scribble/manual

@(require "../Documentations/Handbook/handbook.rkt")

@handbook-title[#:authors '("WarGrey")]{工业数据采集与监控系统手册}

@margin-note{软件工程领域有很多项目文档模版,这里不做比较。
我喜欢像写书一样的来写项目文档,这是@cite{LP:WEB}理念的延续。
不过 C++ 领域没有现成的工具，因此本手册尚不完整，后续有时间我再补全。}

@handbook-purposes[]

@margin-note{本系统所用程序语言及其分工
 @itemlist[#:style 'compact
           @item{@tt{C++} 编写完整的主程序。}
           @item{@tt{@cite{Racket}} 编写所有辅助程序。如：探索系统原型、分析主程序日志、绘制文档图表、转换老代码编码等等。}
           @item{@tt{@cite{Scribble}} 编写本手册。}
           @item{@tt{ResX} 定义主程序界面的中英文字符串，由 Visual Studio 自行维护。}
           @item{@tt{SVG} 定义外部图元。}]}

@handbook-statistics[#:gitstat-width 420 #:gitstat-height 180
 #:ignore '(#px"/Generated[ ]Files/?" #px"/stone/include/")
 #(C++      #xF34B7D #px"[.]([ch](pp|xx)?)$")
 #(Racket   #x89E051 #px"[.]rkt$")
 #(SVG      #xC34C26 #px"[.]svg$")
 #(Scribble #x1E4AEC #px"[.](scrbl|css)$")
 #(ResX     #xCFCFFF #px"[.]resw$")]

@handbook-table[]

@include-section{../Documentations/Handbook/preface.scrbl}
@include-section{../Documentations/Handbook/infrastructure.scrbl}
@include-section{../Documentations/Handbook/architecture.scrbl}
@include-section{../Documentations/Handbook/misc.scrbl}

@handbook-appendix[#:index? #true
 (bib-entry #:key      "SE:APA"
            #:title    "Software Engineering: A Practitioner's Approach"
            #:author   (authors "Roger S. Pressman" "Bruce R. Maxim")
            #:date     "2015"
            #:location (book-location #:edition "Eighth" #:publisher "China Machine Press")
            #:is-book? #true)
 (bib-entry #:key      "GAE"
            #:title    "Game Engine Architecture"
            #:author   (authors "Jason Gregory" "Jeff Lander")
            #:date     "2009"
            #:location (book-location #:edition "First" #:publisher "A K Peters/CRC Press")
            #:is-book? #true)
 (bib-entry #:key      "libmodbus"
            #:title    "A Modbus library for Linux, Mac OS X, FreeBSD, QNX and Windows"
            #:author   (authors "Stéphane Raimbault")
            #:url      "http://libmodbus.org")]
