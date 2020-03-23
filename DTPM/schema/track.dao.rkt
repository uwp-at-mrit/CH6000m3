#lang racket

(require "../../../Toolbox/ORM/schema.rkt")

(define-table track #:as Track #:with [uuid] #:order-by timestamp
  ([uuid          : Integer       #:default pk64_timestamp]
   [type          : Integer       #:not-null]
   [x             : Float         #:not-null]
   [y             : Float         #:not-null]
   [z             : Float         #:not-null]
   [timestamp     : Integer       #:not-null])
  #:include [["dbmisc.hpp"]])
