#lang racket

;;; https://docs.microsoft.com/en-us/windows/uwp/controls-and-patterns/tiles-and-notifications-app-assets

(require racket/draw)
(require images/icons/symbol)

(define rebuer-icon
  (case-lambda
    [(size plate?)
     (recycle-icon #:height size #:backing-scale 1.0)]
    [(width height plate?)
     (cond [(= width height) (rebuer-icon height plate?)]
           [else (let* ([src.png (rebuer-icon (min width height) plate?)]
                        [tile.png (make-bitmap width height #:backing-scale 1.0)])
                   (define dc (send tile.png make-dc))
                   (define offset (/ (- width height) 2))
                   (if (negative? offset)
                       (send dc draw-bitmap src.png 0 (- offset))
                       (send dc draw-bitmap src.png offset 0))
                   tile.png)])]))

(define rebuer.icon*
  (let ([tiles (make-hash)])
    (lambda [width [height #false] #:plate? [plate? #true]]
      (if (or (not height) (= width height))
          (hash-ref! tiles (cons width plate?)
                     (thunk (rebuer-icon width plate?)))
          (hash-ref! tiles (list width height plate?)
                     (thunk (rebuer-icon width height plate?)))))))

(define ~tilename
  (lambda [width height type value plate?]
    (format "~a~ax~aLogo.~a~a-~a.png"
            (if (= width height) 'Square 'Wide) width height
            (if plate? "" "altform-unplated_") type value)))

(define tile-assets-format
  (lambda [width height scale%]
    ;;; Uses:
    ;; Default start tile
    ;; Action center
    ;; Task switcher
    ;; Share picker
    ;; Picker
    ;; Store
    (~tilename width height 'scale scale% #true)))

(define scalable-list-assets-format
  (lambda [scale%]
    ;;; Uses:
    ;; Start all app list
    ;; Start most-frequently used list
    ;; Task manager
    ;; Cortana search results
    ;; Settings
    (~tilename 44 44 'scale scale% #true)))

(define targetsize-list-assets-format
  (lambda [size plate?]
    ;;; Uses when plate?:
    ;; Start jump list
    ;; Start low corner of tile
    ;; Shortcut
    ;; Control Panel
    
    ;;; Uses unless plate:
    ;; Taskbar and taskbar thumbnail
    ;; Taskbar jumplist
    ;; Task view
    ;; ALT-TAB
    (~tilename 44 44 'targetsize size plate?)))

;;; TODO: file-extension-assets-format

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define save-tile
  (lambda [path src.png xscale yscale]
    (define tile.png
      (cond [(= xscale yscale 1.0) src.png]
            [else (let ([width (* (send src.png get-width) xscale)]
                        [height (* (send src.png get-height) yscale)])
                    (define scaled.png (make-bitmap width height #:backing-scale (send src.png get-backing-scale)))
                    (define dc (send scaled.png make-dc))
                    (send dc set-scale xscale yscale)
                    (send dc draw-bitmap src.png 0 0)
                    scaled.png)]))
    (send tile.png save-file path 'png)
    (printf "saved ~a~n" path)))

(define generate-square-tiles
  (lambda [basesize scales description name]
    (displayln description)
    (define maxsize (* basesize (/ (last scales) 100)))
    (define max.png (rebuer.icon* maxsize #:plate? #true))
    (for ([scale% (in-list scales)])
      (define size (exact-round (* basesize (/ scale% 100))))
      (define scale%.path
        (cond [(symbol? name) (format "~a.scale-~a.png" name scale%)]
              [(not name) (tile-assets-format basesize basesize scale%)]
              [else (scalable-list-assets-format scale%)]))
      (define downscale (/ size maxsize))
      (save-tile scale%.path max.png downscale downscale))))

(define generate-rectangle-tiles
  (lambda [base-width base-height scales description name]
    (displayln description)
    (define maxscale (/ (last scales) 100))
    (define mwidth (* base-width maxscale))
    (define mheight (* base-height maxscale))
    (define max.png (rebuer.icon* mwidth mheight #:plate? #true))
    (for ([scale% (in-list scales)])
      (define width (exact-round (* base-width (/ scale% 100))))
      (define height (exact-round (* base-height (/ scale% 100))))
      (define scale%.path
        (cond [(symbol? name) (format "~a.scale-~a.png" name scale%)]
              [else (tile-assets-format base-width base-height scale%)]))
      (save-tile scale%.path max.png (/ width mwidth) (/ height mheight)))))

(define generate-square-icons
  (lambda [sizes description plate?]
    (displayln description)
    (define maxsize (last sizes))
    (define max.png (rebuer.icon* maxsize #:plate? plate?))
    (for ([targetsize (in-list sizes)])
      (define path (targetsize-list-assets-format targetsize plate?))
      (define downscale (/ targetsize maxsize))
      (save-tile path max.png downscale downscale))))

(define generate-application-icons
  (lambda []
    (generate-square-tiles 44 (list 100 125 150 200 400) "[Generating Scalable Application Icons]" #true)
    (generate-square-icons (list 16 24 32 48 256) "[Generating Plated Application Icons]" #true)
    (generate-square-icons (list 16 24 32 48 256) "[Generating Unplated Application Icons]" #false)
    (rename-file-or-directory "Square44x44Logo.altform-unplated_targetsize-24.png"
                              "Square44x44Logo.targetsize-24_altform-unplated.png"
                              #true)))

(define generate-splash-screen
  (lambda []
    (generate-rectangle-tiles 620 300 (list 100 125 150 200 400) "[Generating Splash Screen]" 'SplashScreen)))

(define generate-store-logos
  (lambda []
    (generate-square-tiles 50  (list 100 125 150 200 400) "[Generating Store Logos]" 'StoreLogo)
    (copy-file "StoreLogo.scale-100.png" "StoreLogo.png" #true)))

(define generate-small-tiles
  (lambda []
    (generate-square-tiles 71  (list 100 125 150 200 400) "[Generating Small Tiles]" 'SmallTile)))

(define generate-medium-tiles
  (lambda []
    (generate-square-tiles 150 (list 100 125 150 200 400) "[Generating Medium Tiles]" #false)))

(define generate-large-tiles
  (lambda []
    (generate-square-tiles 310 (list 100 125 150 200 400) "[Generating Large Tiles]" 'LargeTile)))

(define generate-wide-tiles
  (lambda []
    ;;; TODO: there is duplicate work
    (generate-rectangle-tiles 310 150 (list 100 125 150 200 400) "[Generating 350x150 Tiles]" #false)
    (generate-rectangle-tiles 310 150 (list 100 125 150 200 400) "[Generating Wide Tiles]" 'WideTile)))

(module+ main
  (generate-application-icons)
  (generate-splash-screen)
  (generate-store-logos)
  (generate-small-tiles)
  (generate-medium-tiles)
  (generate-large-tiles)
  (generate-wide-tiles))
