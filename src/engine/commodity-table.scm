;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;  commodity-table.scm
;;;  load and save commodity tables 
;;;
;;;  Bill Gribble <grib@billgribble.com> 3 Aug 2000 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(define GNC_COMMODITY_NS_ISO "ISO4217")

;; If you change the C side, change this too.
(define GNC_COMMODITY_NS_AMEX "AMEX")
(define GNC_COMMODITY_NS_NYSE "NYSE")
(define GNC_COMMODITY_NS_NASDAQ "NASDAQ")
(define GNC_COMMODITY_NS_EUREX "EUREX")
(define GNC_COMMODITY_NS_MUTUAL "FUND")

;; Returns true if the commodity comm represents a currency, false if
;; it represents a stock or mutual-fund.
(define (gnc:commodity-is-currency? comm)
  (equal? GNC_COMMODITY_NS_ISO
	  (gnc:commodity-get-namespace comm)))
