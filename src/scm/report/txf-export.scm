;; -*-scheme-*-
;; 
;;  Richard -Gilligan- Uschold
;; These are TXF codes and a brief description of each. See taxtxf.scm
;; and txf-export-help.scm
;;
(gnc:support "report/txf-export.scm")
(gnc:depend "report/txf-export-help.scm")

(define (gnc:txf-get-payer-name-source categories code)
  (gnc:txf-get-code-info categories code 0))
(define (gnc:txf-get-form categories code)
  (gnc:txf-get-code-info categories code 1))
(define (gnc:txf-get-description categories code)
  (gnc:txf-get-code-info categories code 2))
(define (gnc:txf-get-format categories code)
  (gnc:txf-get-code-info categories code 3))
(define (gnc:txf-get-multiple categories code)
  (gnc:txf-get-code-info categories code 4))
(define (gnc:txf-get-help categories code)
  (let ((pair (assv code txf-help-strings)))
    (if pair
        (cdr pair)
        "No help available.")))

(define (gnc:txf-get-codes categories)
  (map car categories))

;;;; Private

(define (gnc:txf-get-code-info categories code index)
  (vector-ref (cdr (assv code categories)) index))

(define txf-help-categories
  (list
   (cons 'H000 #(current "help" "Name of Current account is exported." 0 #f))
   (cons 'H002 #(parent "help" "Name of Parent account is exported." 0 #f))
   (cons 'H003 #(not-impl "help" "Not implemented yet, Do NOT Use!" 0 #f))))

(define txf-income-categories
  (list
   (cons 'N000 #(none "none" "Tax Report Only - No TXF Export" 0 #f))
   (cons 'N261 #(none "F1040" "Alimony received" 1 #f))
   (cons 'N257 #(none "F1040" "Other income, misc." 1 #f))
   (cons 'N520 #(none "F1040" "RR retirement inc., spouse" 1 #f))
   (cons 'N519 #(none "F1040" "RR retirement income, self" 1 #f))
   (cons 'N258 #(none "F1040" "Sick pay or disability pay" 1 #f))
   (cons 'N483 #(none "F1040" "Social Security inc., spouse" 1 #f))
   (cons 'N266 #(none "F1040" "Social Security income, self" 1 #f))
   (cons 'N260 #(none "F1040" "State and local tax refunds" 1 #t))
   (cons 'N269 #(none "F1040" "Taxable fringe benefits" 1 #f))
   (cons 'N672 #(none "F1099-G" "Qualified state tuition earnings" 1 #t))
   (cons 'N260 #(none "F1099-G" "State and local tax refunds" 1 #t))
   (cons 'N479 #(none "F1099-G" "Unemployment compensation" 1 #t))
   (cons 'N562 #(none "F1099-MISC" "Crop insurance proceeds" 1 #t))
   (cons 'N559 #(none "F1099-MISC" "Fishing boat proceeds" 1 #t))
   (cons 'N560 #(none "F1099-MISC" "Medical/health payments" 1 #t))
   (cons 'N561 #(none "F1099-MISC" "Nonemployee compensation" 1 #t))
   (cons 'N557 #(none "F1099-MISC" "Other income" 1 #t))
   (cons 'N259 #(none "F1099-MISC" "Prizes and awards" 1 #t))
   (cons 'N555 #(none "F1099-MISC" "Rents" 1 #t))
   (cons 'N556 #(none "F1099-MISC" "Royalties" 1 #t))
   (cons 'N632 #(none "F1099-MSA" "MSA earnings on excess contrib" 1 #t))
   (cons 'N631 #(none "F1099-MSA" "MSA gross distribution" 1 #t))
   (cons 'N623 #(none "F1099-R" "SIMPLE total gross distrib." 1 #t))
   (cons 'N624 #(none "F1099-R" "SIMPLE total taxable distrib." 1 #t))
   (cons 'N477 #(none "F1099-R" "Total IRA gross distrib." 1 #t))
   (cons 'N478 #(none "F1099-R" "Total IRA taxable distrib." 1 #t))
   (cons 'N475 #(none "F1099-R" "Total pension gross distrib." 1 #t))
   (cons 'N476 #(none "F1099-R" "Total pension taxable dist." 1 #t))
   (cons 'N387 #(none "F2106" "Reimb. exp. (non-meal/ent.)" 1 #t))
   (cons 'N388 #(none "F2106" "Reimb. meal/entertainment exp." 1 #t))
   (cons 'N505 #(none "F4137" "Total cash/tips received" 1 #t))
   (cons 'N416 #(none "F4684" "FMV after casualty" 1 #t))
   (cons 'N415 #(none "F4684" "FMV before casualty" 1 #t))
   (cons 'N414 #(none "F4684" "Insurance/reimbursement" 1 #t))
   (cons 'N573 #(none "F4835" "Agricultural program paymnts" 1 #t))
   (cons 'N575 #(none "F4835" "CCC loans forfeited/repaid" 1 #t))
   (cons 'N574 #(none "F4835" "CCC loans reported/election" 1 #t))
   (cons 'N577 #(none "F4835" "Crop ins. proceeds deferred" 1 #t))
   (cons 'N576 #(none "F4835" "Crop ins. proceeds rec'd" 1 #t))
   (cons 'N578 #(none "F4835" "Other income" 1 #t))
   (cons 'N571 #(none "F4835" "Sale of livestock/produce" 1 #t))
   (cons 'N572 #(none "F4835" "Total co-op distributions" 1 #t))
   (cons 'N429 #(none "F6252" "Debt assumed by buyer" 1 #t))
   (cons 'N431 #(none "F6252" "Depreciation allowed" 1 #t))
   (cons 'N435 #(none "F6252" "Payments rec'd prior years" 1 #t))
   (cons 'N434 #(none "F6252" "Payments rec'd this year" 1 #t))
   (cons 'N428 #(none "F6252" "Selling price" 1 #t))
   (cons 'N444 #(none "F8815" "EE US svgs. bonds proceeds" 1 #f))
   (cons 'N443 #(none "F8815" "Nontaxable ed. benefits" 1 #f))
   (cons 'N445 #(none "F8815" "Post-89 EE bond face value" 1 #f))
   (cons 'N637 #(none "F8863" "Hope credit" 1 #t))
   (cons 'N638 #(none "F8863" "Lifetime learning credit" 1 #t))
   (cons 'N393 #(none "Home Sale" "Selling price of old home" 1 #t))
   (cons 'N488 #(parent "Sched B" "Div. income, cap gain distrib." 3 #t))
   (cons 'N487 #(current "Sched B" "Div. income, non-taxable" 3 #t))
   (cons 'N286 #(parent "Sched B" "Dividend income" 3 #t))
   (cons 'N492 #(current "Sched B" "Int. inc., OID bonds" 3 #t))
   (cons 'N490 #(current "Sched B" "Int. inc., taxed only by fed" 3 #t))
   (cons 'N491 #(current "Sched B" "Int. inc., taxed only by state" 3 #t))
   (cons 'N489 #(current "Sched B" "Int. income, non-taxable" 3 #t))
   (cons 'N287 #(current "Sched B" "Interest income" 3 #t))
   (cons 'N524 #(current "Sched B" "Seller-financed mortgage int." 3 #t))
   (cons 'N289 #(current "Sched B" "State and muni bond interest" 3 #t))
   (cons 'N290 #(current "Sched B" "TE private activity bond int." 3 #t))
   (cons 'N288 #(current "Sched B" "US government interest" 3 #t))
   (cons 'N293 #(none "Sched C" "Gross receipts or sales" 1 #t))
   (cons 'N303 #(none "Sched C" "Other business income" 1 #t))
   (cons 'N323 #(not-impl "Sched D" "LT gain/loss - security" 1 #f))
   (cons 'N321 #(not-impl "Sched D" "ST gain/loss - security" 1 #f))
   (cons 'N810 #(not-impl "Sched D" "ST/LT gain or loss" 1 #f))

   (cons 'N326 #(none "Sched E" "Rents received" 1 #t))
   (cons 'N327 #(none "Sched E" "Royalties received" 1 #t))
   (cons 'N372 #(none "Sched F" "Agricultural program payments" 1 #t))
   (cons 'N374 #(none "Sched F" "CCC loans forfeited or repaid" 1 #t))
   (cons 'N373 #(none "Sched F" "CCC loans reported/election" 1 #t))
   (cons 'N376 #(none "Sched F" "Crop ins. proceeds deferred" 1 #t))
   (cons 'N375 #(none "Sched F" "Crop ins. proceeds received" 1 #t))
   (cons 'N370 #(none "Sched F" "Custom hire income" 1 #t))
   (cons 'N377 #(none "Sched F" "Other farm income" 1 #t))
   (cons 'N369 #(none "Sched F" "Resales of livestock/items" 1 #t))
   (cons 'N368 #(none "Sched F" "Sales livestock/product raised" 1 #t))
   (cons 'N371 #(none "Sched F" "Total co-op. distributions" 1 #t))
   (cons 'N452 #(none "Sched K-1" "Dividends" 1 #t))
   (cons 'N455 #(none "Sched K-1" "Guaranteed partner payments" 1 #t))
   (cons 'N451 #(none "Sched K-1" "Interest income" 1 #t))

   (cons 'N454 #(not-impl "Sched K-1" "Net LT capital gain or loss" 1 #t))
   (cons 'N453 #(not-impl "Sched K-1" "Net ST capital gain or loss" 1 #t))
   (cons 'N456 #(not-impl "Sched K-1" "Net Section 1231 gain or loss" 1 #t))
   (cons 'N448 #(not-impl "Sched K-1" "Ordinary income or loss" 1 #t))
   (cons 'N450 #(not-impl "Sched K-1" "Other rental income or loss" 1 #t))
   (cons 'N449 #(not-impl "Sched K-1" "Rental RE income or loss" 1 #t))

   (cons 'N527 #(none "Sched K-1" "Royalties" 1 #t))
   (cons 'N528 #(none "Sched K-1" "Tax-exempt interest income" 1 #t))
   (cons 'N465 #(none "W-2" "Dep. care benefits, self" 1 #t))
   (cons 'N512 #(none "W-2" "Dep. care benefits, spouse" 1 #t))
   (cons 'N546 #(none "W-2" "Reimbursed moving exp.,spouse" 1 #t))
   (cons 'N267 #(none "W-2" "Reimbursed moving expenses" 1 #t))
   (cons 'N460 #(none "W-2" "Salary or wages, self" 1 #t))
   (cons 'N506 #(none "W-2" "Salary or wages, spouse" 1 #t))
   (cons 'N549 #(none "W-2G" "Gross winnings" 1 #t))))

(define txf-expense-categories
  (list
   (cons 'N000 #(none "none" "Tax Report Only - No TXF Export" 0 #f))
   (cons 'N264 #(none "F1040" "Alimony paid" 1 #f))
   (cons 'N265 #(current "F1040" "Early withdrawal penalty" 3 #f))
   (cons 'N521 #(none "F1040" "Federal estimated tax, qrtrly" 1 #f))
   (cons 'N613 #(none "F1040" "Fed tax w/h, RR retire,self" 1 #f))
   (cons 'N611 #(none "F1040" "Fed tax w/h, Soc. Sec.,self" 1 #f))
   (cons 'N614 #(none "F1040" "Fed tax w/h,RR retire,spouse" 1 #f))
   (cons 'N612 #(none "F1040" "Fed tax w/h,Soc. Sec.,spouse" 1 #f))
   (cons 'N482 #(none "F1040" "IRA contrib., non-work spouse" 1 #f))
   (cons 'N262 #(none "F1040" "IRA contribution, self" 1 #f))
   (cons 'N481 #(none "F1040" "IRA contribution, spouse" 1 #f))
   (cons 'N263 #(none "F1040" "Keogh deduction, self" 1 #f))
   (cons 'N516 #(none "F1040" "Keogh deduction, spouse" 1 #f))
   (cons 'N608 #(none "F1040" "Med savings contrib., spouse" 1 #f))
   (cons 'N607 #(none "F1040" "Med savings contribution,self" 1 #f))
   (cons 'N517 #(none "F1040" "SEP deduction, self" 1 #f))
   (cons 'N518 #(none "F1040" "SEP deduction, spouse" 1 #f))
   (cons 'N609 #(none "F1040" "SIMPLE contribution, self" 1 #f))
   (cons 'N610 #(none "F1040" "SIMPLE contribution, spouse" 1 #f))
   (cons 'N636 #(none "F1040" "Student loan interest" 1 #f))
   (cons 'N606 #(none "F1099-G" "Fed tax w/h, unemplymt comp" 1 #t))
   (cons 'N605 #(none "F1099-G" "Unemployment comp repaid" 1 #t))
   (cons 'N558 #(none "F1099-MISC" "Federal tax withheld" 1 #t))
   (cons 'N563 #(none "F1099-MISC" "State tax withheld" 1 #t))
   (cons 'N532 #(none "F1099-R" "IRA federal tax withheld" 1 #t))
   (cons 'N534 #(none "F1099-R" "IRA local tax withheld" 1 #t))
   (cons 'N533 #(none "F1099-R" "IRA state tax withheld" 1 #t))
   (cons 'N529 #(none "F1099-R" "Pension federal tax withheld" 1 #t))
   (cons 'N531 #(none "F1099-R" "Pension local tax withheld" 1 #t))
   (cons 'N530 #(none "F1099-R" "Pension state tax withheld" 1 #t))
   (cons 'N625 #(none "F1099-R" "SIMPLE federal tax withheld" 1 #t))
   (cons 'N627 #(none "F1099-R" "SIMPLE local tax withheld" 1 #t))
   (cons 'N626 #(none "F1099-R" "SIMPLE state tax withheld" 1 #t))
   (cons 'N382 #(none "F2106" "Automobile expenses" 1 #t))
   (cons 'N381 #(none "F2106" "Education expenses" 1 #t))
   (cons 'N391 #(none "F2106" "Employee home office expenses" 1 #t))
   (cons 'N389 #(none "F2106" "Job seeking expenses" 1 #t))
   (cons 'N384 #(none "F2106" "Local transportation exp." 1 #t))
   (cons 'N386 #(none "F2106" "Meal/entertainment expenses" 1 #t))
   (cons 'N385 #(none "F2106" "Other business expenses" 1 #t))
   (cons 'N390 #(none "F2106" "Special clothing expenses" 1 #t))
   (cons 'N383 #(none "F2106" "Travel (away from home)" 1 #t))
   (cons 'N401 #(current "F2441" "Qualifying childcare expenses" 3 #f))
   (cons 'N402 #(current "F2441" "Qualifying household expenses" 3 #f))
   (cons 'N406 #(none "F3903" "Transport/storage of goods" 1 #t))
   (cons 'N407 #(none "F3903" "Travel/lodging, except meals" 1 #t))
   (cons 'N413 #(none "F4684" "Basis of casualty property" 1 #t))
   (cons 'N579 #(none "F4835" "Car and truck expenses" 1 #t))
   (cons 'N580 #(none "F4835" "Chemicals" 1 #t))
   (cons 'N581 #(none "F4835" "Conservation expenses" 1 #t))
   (cons 'N582 #(none "F4835" "Custom hire expenses" 1 #t))
   (cons 'N583 #(none "F4835" "Employee benefit programs" 1 #t))
   (cons 'N584 #(none "F4835" "Feed purchased" 1 #t))
   (cons 'N585 #(none "F4835" "Fertilizers and lime" 1 #t))
   (cons 'N586 #(none "F4835" "Freight and trucking" 1 #t))
   (cons 'N587 #(none "F4835" "Gasoline, fuel, and oil" 1 #t))
   (cons 'N588 #(none "F4835" "Insurance (other than health)" 1 #t))
   (cons 'N589 #(none "F4835" "Interest expense, mortgage" 1 #t))
   (cons 'N590 #(none "F4835" "Interest expense, other" 1 #t))
   (cons 'N591 #(none "F4835" "Labor hired" 1 #t))
   (cons 'N602 #(none "F4835" "Other expenses" 1 #t))
   (cons 'N592 #(none "F4835" "Pension/profit-sharing plans" 1 #t))
   (cons 'N594 #(none "F4835" "Rent/lease land, animals" 1 #t))
   (cons 'N593 #(none "F4835" "Rent/lease vehicles, equip." 1 #t))
   (cons 'N595 #(none "F4835" "Repairs and maintenance" 1 #t))
   (cons 'N596 #(none "F4835" "Seeds and plants purchased" 1 #t))
   (cons 'N597 #(none "F4835" "Storage and warehousing" 1 #t))
   (cons 'N598 #(none "F4835" "Supplies purchased" 1 #t))
   (cons 'N599 #(none "F4835" "Taxes" 1 #t))
   (cons 'N600 #(none "F4835" "Utilities" 1 #t))
   (cons 'N601 #(none "F4835" "Vet, breeding, medicine" 1 #t))
   (cons 'N426 #(none "F4952" "Investment interest expense" 1 #f))
   (cons 'N432 #(none "F6252" "Expenses of sale" 1 #t))
   (cons 'N442 #(none "F8815" "Qual. higher ed. expenses" 1 #f))
   (cons 'N537 #(none "F8829" "Deductible mortgage expense" 1 #t))
   (cons 'N539 #(none "F8829" "Insurance" 1 #t))
   (cons 'N542 #(none "F8829" "Other expenses" 1 #t))
   (cons 'N538 #(none "F8829" "Real estate taxes" 1 #t))
   (cons 'N540 #(none "F8829" "Repairs and maintenance" 1 #t))
   (cons 'N541 #(none "F8829" "Utilities" 1 #t))
   (cons 'N618 #(none "F8839" "Adoption fees" 1 #t))
   (cons 'N620 #(none "F8839" "Attorney fees" 1 #t))
   (cons 'N619 #(none "F8839" "Court costs" 1 #t))
   (cons 'N622 #(none "F8839" "Other expenses" 1 #t))
   (cons 'N621 #(none "F8839" "Traveling expenses" 1 #t))
   (cons 'N397 #(none "Home Sale" "Cost of new home" 1 #t))
   (cons 'N394 #(none "Home Sale" "Expense of sale" 1 #t))
   (cons 'N396 #(none "Home Sale" "Fixing-up expenses" 1 #t))
   (cons 'N280 #(none "Sched A" "Cash charity contributions" 1 #f))
   (cons 'N484 #(none "Sched A" "Doctors, dentists, hospitals" 1 #f))
   (cons 'N272 #(none "Sched A" "Gambling losses" 1 #f))
   (cons 'N545 #(none "Sched A" "Home mortgage int. (no 1098)" 1 #f))
   (cons 'N283 #(none "Sched A" "Home mortgage interest (1098)" 1 #f))
   (cons 'N282 #(none "Sched A" "Investment management fees" 1 #f))
   (cons 'N544 #(none "Sched A" "Local income taxes" 1 #f))
   (cons 'N274 #(none "Sched A" "Medical travel and lodging" 1 #f))
   (cons 'N273 #(none "Sched A" "Medicine and drugs" 1 #f))
   (cons 'N523 #(none "Sched A" "Misc., no 2% AGI limit" 1 #f))
   (cons 'N486 #(none "Sched A" "Misc., subject to 2% AGI limit" 1 #f))
   (cons 'N485 #(none "Sched A" "Non-cash charity contributions" 1 #f))
   (cons 'N277 #(none "Sched A" "Other taxes" 1 #f))
   (cons 'N535 #(none "Sched A" "Personal property taxes" 1 #f))
   (cons 'N284 #(none "Sched A" "Points paid (no 1098)" 1 #f))
   (cons 'N276 #(none "Sched A" "Real estate taxes" 1 #f))
   (cons 'N522 #(none "Sched A" "State estimated tax, qrtrly" 1 #f))
   (cons 'N275 #(none "Sched A" "State income taxes" 1 #f))
   (cons 'N271 #(none "Sched A" "Subscriptions" 1 #f))
   (cons 'N615 #(current "Sched B" "Fed tax w/h, dividend income" 3 #t))
   (cons 'N616 #(current "Sched B" "Fed tax w/h, interest income" 3 #t))
   (cons 'N281 #(none "Sched A" "Tax preparation fees" 1 #f))
   (cons 'N304 #(none "Sched C" "Advertising" 1 #t))
   (cons 'N305 #(none "Sched C" "Bad debts from sales/services" 1 #t))
   (cons 'N306 #(none "Sched C" "Car and truck expenses" 1 #t))
   (cons 'N307 #(none "Sched C" "Commissions and fees" 1 #t))
   (cons 'N309 #(none "Sched C" "Depletion" 1 #t))
   (cons 'N308 #(none "Sched C" "Employee benefit programs" 1 #t))
   (cons 'N310 #(none "Sched C" "Insurance, other than health" 1 #t))
   (cons 'N311 #(none "Sched C" "Interest expense, mortgage" 1 #t))
   (cons 'N312 #(none "Sched C" "Interest expense, other" 1 #t))
   (cons 'N494 #(none "Sched C" "Labor, Cost of Goods" 1 #t))
   (cons 'N298 #(none "Sched C" "Legal and professional fees" 1 #t))
   (cons 'N495 #(none "Sched C" "Materials/supplies, COGS" 1 #t))
   (cons 'N294 #(none "Sched C" "Meals and entertainment" 1 #t))
   (cons 'N313 #(none "Sched C" "Office expenses" 1 #t))
   (cons 'N302 #(none "Sched C" "Other business expenses" 1 #t))
   (cons 'N496 #(none "Sched C" "Other costs, COGS" 1 #t))
   (cons 'N314 #(none "Sched C" "Pension/profit sharing plans" 1 #t))
   (cons 'N493 #(none "Sched C" "Purchases, Cost of Goods" 1 #t))
   (cons 'N300 #(none "Sched C" "Rent/lease other bus. prop." 1 #t))
   (cons 'N299 #(none "Sched C" "Rent/lease vehicles, equip." 1 #t))
   (cons 'N315 #(none "Sched C" "Repairs and maintenance" 1 #t))
   (cons 'N296 #(none "Sched C" "Returns and allowances" 1 #t))
   (cons 'N301 #(none "Sched C" "Supplies (not from COGS)" 1 #t))
   (cons 'N316 #(none "Sched C" "Taxes and licenses" 1 #t))
   (cons 'N317 #(none "Sched C" "Travel" 1 #t))
   (cons 'N318 #(none "Sched C" "Utilities" 1 #t))
   (cons 'N297 #(none "Sched C" "Wages paid" 1 #t))
   (cons 'N328 #(none "Sched E" "Advertising" 1 #t))
   (cons 'N329 #(none "Sched E" "Auto and travel" 1 #t))
   (cons 'N330 #(none "Sched E" "Cleaning and maintenance" 1 #t))
   (cons 'N331 #(none "Sched E" "Commissions" 1 #t))
   (cons 'N332 #(none "Sched E" "Insurance" 1 #t))
   (cons 'N333 #(none "Sched E" "Legal and professional fees" 1 #t))
   (cons 'N502 #(none "Sched E" "Management fees" 1 #t))
   (cons 'N334 #(none "Sched E" "Mortgage interest expense" 1 #t))
   (cons 'N341 #(none "Sched E" "Other expenses" 1 #t))
   (cons 'N335 #(none "Sched E" "Other interest expense" 1 #t))
   (cons 'N336 #(none "Sched E" "Repairs" 1 #t))
   (cons 'N337 #(none "Sched E" "Supplies" 1 #t))
   (cons 'N338 #(none "Sched E" "Taxes" 1 #t))
   (cons 'N339 #(none "Sched E" "Utilities" 1 #t))
   (cons 'N543 #(none "Sched F" "Car and truck expenses" 1 #t))
   (cons 'N366 #(none "Sched F" "Chemicals" 1 #t))
   (cons 'N362 #(none "Sched F" "Conservation expenses" 1 #t))
   (cons 'N378 #(none "Sched F" "Cost of resale livestock/items" 1 #t))
   (cons 'N367 #(none "Sched F" "Custom hire expenses" 1 #t))
   (cons 'N364 #(none "Sched F" "Employee benefit programs" 1 #t))
   (cons 'N350 #(none "Sched F" "Feed purchased" 1 #t))
   (cons 'N352 #(none "Sched F" "Fertilizers and lime" 1 #t))
   (cons 'N361 #(none "Sched F" "Freight and trucking" 1 #t))
   (cons 'N356 #(none "Sched F" "Gasoline, fuel, and oil" 1 #t))
   (cons 'N359 #(none "Sched F" "Insurance, other than health" 1 #t))
   (cons 'N346 #(none "Sched F" "Interest expense, mortgage" 1 #t))
   (cons 'N347 #(none "Sched F" "Interest expense, other" 1 #t))
   (cons 'N344 #(none "Sched F" "Labor hired" 1 #t))
   (cons 'N365 #(none "Sched F" "Other farm expenses" 1 #t))
   (cons 'N363 #(none "Sched F" "Pension/profit sharing plans" 1 #t))
   (cons 'N348 #(none "Sched F" "Rent/lease land, animals" 1 #t))
   (cons 'N349 #(none "Sched F" "Rent/lease vehicles, equip." 1 #t))
   (cons 'N345 #(none "Sched F" "Repairs and maintenance" 1 #t))
   (cons 'N351 #(none "Sched F" "Seeds and plants purchased" 1 #t))
   (cons 'N357 #(none "Sched F" "Storage and warehousing" 1 #t))
   (cons 'N353 #(none "Sched F" "Supplies purchased" 1 #t))
   (cons 'N358 #(none "Sched F" "Taxes" 1 #t))
   (cons 'N360 #(none "Sched F" "Utilities" 1 #t))
   (cons 'N355 #(none "Sched F" "Vet, breeding, and medicine" 1 #t))
   (cons 'N567 #(none "Sched H" "Cash wages paid" 1 #f))
   (cons 'N568 #(none "Sched H" "Federal tax withheld" 1 #f))
   (cons 'N461 #(none "W-2" "Federal tax withheld, self" 1 #t))
   (cons 'N507 #(none "W-2" "Federal tax withheld, spouse" 1 #t))
   (cons 'N463 #(none "W-2" "Local tax withheld, self" 1 #t))
   (cons 'N509 #(none "W-2" "Local tax withheld, spouse" 1 #t))
   (cons 'N480 #(none "W-2" "Medicare tax withheld, self" 1 #t))
   (cons 'N510 #(none "W-2" "Medicare tax withheld, spouse" 1 #t))
   (cons 'N462 #(none "W-2" "Soc. Sec. tax withheld, self" 1 #t))
   (cons 'N508 #(none "W-2" "Soc. Sec. tax withheld, spouse" 1 #t))
   (cons 'N464 #(none "W-2" "State tax withheld, self" 1 #t))
   (cons 'N511 #(none "W-2" "State tax withheld, spouse" 1 #t))
   (cons 'N550 #(none "W-2G" "Federal tax withheld" 1 #t))
   (cons 'N551 #(none "W-2G" "State tax withheld" 1 #t))))
