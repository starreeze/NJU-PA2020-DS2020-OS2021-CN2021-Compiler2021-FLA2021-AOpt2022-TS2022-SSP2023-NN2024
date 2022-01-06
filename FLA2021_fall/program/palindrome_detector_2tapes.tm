; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, eg '1001001'

; the finite set of states
#Q = {0,cp,cmp,mh,accept,accept2,accept3,accept4,halt_accept,reject,reject2,reject3,reject4,reject5,halt_reject}

; the finite set of input symbols
#S = {0,1}

; the complete set of tape symbols
#G = {0,1,_,T,r,u,e,F,a,l,s}

; the start state
#q0 = 0

; the blank symbol
#B = _

; the set of final states
#F = {halt_accept}

; the number of tapes
#N = 2 

; the transition functions

; State 0: begin
0 ** ** ** cp
0 __ __ ** accept     ; Empty input

; State cp: copy the string to the 2nd tape 
cp 0_ 00 rr cp
cp 1_ 11 rr cp
cp __ __ ll mh


; State mh: move 1st head to the left
mh _* _* r* cmp
mh ** ** l* mh 

; State cmp: compare two strings
cmp 00 __ rl cmp
cmp 11 __ rl cmp
cmp 01 __ rl reject
cmp 10 __ rl reject
cmp __ __ ** accept

accept __ T_ r* accept2
accept2 __ r_ r* accept3
accept3 __ u_ r* accept4
accept4 __ e_ ** halt_accept

reject ** __ rl reject
reject __ F_ r* reject2
reject2 __ a_ r* reject3
reject3 __ l_ r* reject4
reject4 __ s_ r* reject5
reject5 __ e_ ** halt_reject