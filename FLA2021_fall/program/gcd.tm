; the finite set of states
#Q = {f0,mv,mh1,mh2,mh3,dif,mh2l,mv12,mv122,mh2r,mh22l,mh1l2,mh1l3,cp21,mh1l,mv21,mv212,mh1r,mh12l,mh2l2,mh2l3,cp12,fb,halt}

; the finite set of input symbols
#S = {0,1}

; the complete set of tape symbols
#G = {0,1,2,_}

; the start state
#q0 = f0

; the blank symbol
#B = _

; the set of final states
#F = {halt}

; the number of tapes
#N = 2 

; the transition functions

; State f0: begin, find the first 0
f0 1* ** r* f0
f0 0* _* r* mv

; State mv: move the right string to the 2nd tape 
mv 0_ _0 rr mv
mv 1_ _1 rr mv
mv __ __ ll mh1

; State mh1: move 1st head to the last '1'
mh1 1* 1* ** mh2
mh1 _* _* l* mh1

; State mh2: move 1st head to the leftmost & mark 0 to the leftmost
mh2 1* 1* l* mh2
mh2 _* 0* r* mh3

; State mh3: move 2nd head to the leftmost & mark
mh3 *1 *1 *l mh3
mh3 *_ *0 *r dif

; State dif: calculate difference of two strings
dif 11 __ rr dif
dif 1_ 10 *l mh2l
dif _1 01 l* mh1l
dif __ __ l* fb

; **************** 1st > 2nd *****************
; State mh2l: move 2nd head left until 0
mh2l *_ *_ *l mh2l
mh2l *0 *0 *r mv12

; State mv12: move the right string of 1st tape to the 2nd tape
mv12 1_ _1 rr mv12
mv12 10 _2 rr mv122
mv12 _* _* l* mh2r

; State mv122: after seeing 2, continue
mv122 1_ _1 rr mv122
mv122 __ __ ll mh22l

; State mh2r move 2nd head right until 0
mh2r *_ *_ *r mh2r
mh2r *0 *_ *l mh1l2

; State mh22l move 2nd head left until 2
mh22l *1 *1 *l mh22l
mh22l *2 *1 *l mh1l2

; State mh1l2: move 1st head left until 0
mh1l2 _* _* l* mh1l2
mh1l2 0* 0* r* cp21

; State cp21: move 2nd head left until 0 while fill 1st tape right with 1
cp21 _* 1* rl cp21
cp21 _0 _0 lr mh1l3

; State mh1l3: move 1st head until 0
mh1l3 1* 1* l* mh1l3
mh1l3 0* 0* r* dif

; **************** 1st < 2nd *****************
; State mh1l: move 1nd head left until 0
mh1l _* _* l* mh1l
mh1l 0* 0* r* mv21

; State mv21: move the right string of 2nd tape to the 1st tape 
mv21 _1 1_ rr mv21
mv21 01 2_ rr mv212
mv21 *_ *_ *l mh1r

; State mv212: after seeing 2, continue
mv212 _1 1_ rr mv212
mv212 __ __ ll mh12l

; State mh1r move 1st head right until 0
mh1r _* _* r* mh1r
mh1r 0* _* l* mh2l2

; State mh12l move 1st head left until 2
mh12l 1* 1* l* mh12l
mh12l 2* 1* l* mh2l2

; State mh2l2: move 2nd head left until 0
mh2l2 *_ *_ *l mh2l2
mh2l2 *0 *0 *r cp12

; State cp12: move 1st head left until 0 while fill 2nd tape right with 1
cp12 *_ *1 lr cp12
cp12 0_ 0_ rl mh2l3

; State mh2l3: move 1nd head until 0
mh2l3 *1 *1 *l mh2l3
mh2l3 *0 *0 *r dif

; ****************** accept *******************
; State fb fill back on 1st tape
fb _* 1* l* fb
fb 0* _* r* halt
