#lang racket

(require "../../Scripts/ORM/schema.rkt")

(define-table earthwork #:as EarthWork #:with [uuid] #:order-by timestamp
  ([uuid          : Integer       #:default pk64_timestamp]
   [product       : Float         #:not-null]
   [vessel        : Float         #:not-null]
   [hopper_height : Float         #:not-null]
   [loading       : Float         #:not-null]
   [displacement  : Float         #:not-null]
   [timestamp     : Integer       #:not-null #:unique])
  #:include [["dbmisc.hpp"]])
