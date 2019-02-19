#lang racket

(require "../../Scripts/ORM/schema.rkt")

(define-table alarm #:as Alarm #:with [uuid] #:order-by alarmtime
  ([uuid          : Integer       #:default pk64_timestamp]
   [index         : Integer       #:not-null]
   [type          : Integer       #:default 1000]
   [alarmtime     : Integer       #:not-null]
   [fixedtime     : Integer       #:default 0])
  #:include [["dbmisc.hpp"]])
