;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; price-scatter.scm: A scatter plot report about some price.
;;
;; By Christian Stimming <stimming@tuhh.de>
;;
;; This program is free software; you can redistribute it and/or    
;; modify it under the terms of the GNU General Public License as   
;; published by the Free Software Foundation; either version 2 of   
;; the License, or (at your option) any later version.              
;;                                                                  
;; This program is distributed in the hope that it will be useful,  
;; but WITHOUT ANY WARRANTY; without even the implied warranty of   
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    
;; GNU General Public License for more details.                     
;;                                                                  
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, contact:
;;
;; Free Software Foundation           Voice:  +1-617-542-5942
;; 59 Temple Place - Suite 330        Fax:    +1-617-542-2652
;; Boston, MA  02111-1307,  USA       gnu@gnu.org
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(gnc:support "report/price-scatter.scm")
(gnc:depend  "report-html.scm")

(let ((optname-from-date (N_ "From"))
      (optname-to-date (N_ "To"))
      (optname-stepsize (N_ "Step Size"))
      (optname-report-currency (N_ "Report's currency"))

      (optname-accounts (N_ "Accounts"))

      (optname-inc-exp (N_ "Show Income/Expense"))
      (optname-show-profit (N_ "Show Net Profit"))

      (optname-sep-bars (N_ "Show Asset & Liability bars"))
      (optname-net-bars (N_ "Show Net Worth bars"))

      (optname-marker (N_ "Marker"))
      (optname-markercolor (N_ "Marker Color"))
      (optname-plot-width (N_ "Plot Width"))
      (optname-plot-height (N_ "Plot Height")))

  (define (options-generator)
    (let* ((options (gnc:new-options)) 
           ;; This is just a helper function for making options.
           (add-option 
            (lambda (new-option)
              (gnc:register-option options new-option))))

      (gnc:options-add-date-interval!
       options gnc:pagename-general
       optname-from-date optname-to-date "a")

      (gnc:options-add-interval-choice! 
       options gnc:pagename-general optname-stepsize "b" 'MonthDelta)

      (add-option
       (gnc:make-account-list-option
	gnc:pagename-accounts optname-accounts
	"c"
	(N_ "Report on these accounts, if chosen account level allows.")
	(lambda ()
	  (gnc:group-get-subaccounts (gnc:get-current-group)))
	(lambda (accounts)
	  (list #t
		accounts))
	#t))

      (gnc:options-add-currency! 
       options gnc:pagename-general optname-report-currency "d")
      
      (gnc:options-add-plot-size! 
       options gnc:pagename-display 
       optname-plot-width optname-plot-height "c" 500 400)

;      (add-option
;       (gnc:make-multichoice-option
;	gnc:pagename-display optname-marker
;	"a"
;	(N_ "Choose a marker")
;	"cross"
;	(list
;	 (vector "circle" "circle" "circle")
;	 (vector "cross" "cross" "cross")
;	 (vector "square" "square" "square")
;	 (vector "asterisk" "asterisk" "asterisk")
;	 (vector "filled circle" "filled circle" "filled circle")
;	 (vector "filled square" "filled square" "filled square"))))

;      (add-option
;       (gnc:make-color-option
;	gnc:pagename-display optname-markercolor
;	"b"
;	(N_ "Color of the marker")
;	(list #xb2 #x22 #x22 0)
;	255 #f))

      (gnc:options-set-default-section options gnc:pagename-general)

      options))
  
  ;;;;;;;;;;;;;;;;;;;;;;;;
  ;; The renderer function
  (define (renderer report-obj)

    ;; This is a helper function for looking up option values.
    (define (op-value section name)
      (gnc:option-value 
       (gnc:lookup-option (gnc:report-options report-obj) section name)))

    (let* ((to-date-tp (gnc:timepair-end-day-time 
			(gnc:date-option-absolute-time
                         (op-value gnc:pagename-general 
				   optname-to-date))))
	   (from-date-tp (gnc:timepair-start-day-time 
			  (gnc:date-option-absolute-time
                           (op-value gnc:pagename-general 
				     optname-from-date))))
	   (interval (op-value gnc:pagename-general optname-stepsize))
	   (accounts (op-value gnc:pagename-accounts optname-accounts))

	   (height (op-value gnc:pagename-display optname-plot-height))
	   (width (op-value gnc:pagename-display optname-plot-width))
	   ;;(marker (op-value gnc:pagename-display optname-marker))
;	   (mcolor 
;	    (gnc:color-option->hex-string
;	     (gnc:lookup-option (gnc:report-options report-obj)
;				gnc:pagename-display optname-markercolor)))
	   
           (report-currency (op-value gnc:pagename-general
                                      optname-report-currency))

	   (dates-list (gnc:make-date-list
			(gnc:timepair-end-day-time from-date-tp) 
                        (gnc:timepair-end-day-time to-date-tp)
                        (eval interval)))
	   
	   (document (gnc:make-html-document))
	   (chart (gnc:make-html-scatter)))
      
      (gnc:html-scatter-set-title! 
       chart (_ "Price Plot (Test)"))
      (gnc:html-scatter-set-subtitle!
       chart (sprintf #f
                      (_ "%s to %s")
                      (gnc:timepair-to-datestring from-date-tp) 
                      (gnc:timepair-to-datestring to-date-tp)))
      (gnc:html-scatter-set-width! chart width)
      (gnc:html-scatter-set-height! chart height)
      ;;(warn marker mcolor)
      ;;(gnc:html-scatter-set-marker! chart marker)
      ;;(gnc:html-scatter-set-markercolor! chart mcolor)
      (gnc:html-scatter-set-y-axis-label!
       chart (gnc:commodity-get-mnemonic report-currency))

      (gnc:html-scatter-set-data! 
       chart
       '((1.0 1.0) (1.1 1.2) (1.2 1.4) (1.3 1.6) 
	 (2.0 1.0) (2.1 1.2) (2.2 1.4) (2.3 1.6)))

      (gnc:html-document-add-object! document chart) 

      document))

  ;; Here we define the actual report
  (gnc:define-report
   'version 1
   'name (N_ "Price Scatter Plot (Test)")
   ;;'menu-path (list gnc:menuname-asset-liability)
   'options-generator options-generator
   'renderer renderer))

