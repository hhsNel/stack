	MOVrw Rrdi__ x1000_
	CALL_ @mmaps
	MOVrr Rrdi__ Rrax__
	MOVrw Rrsi__ &label
	MOVrw Rrdx__ x000c_
	CALL_ @memcp
	MOVrr Rrsi__ Rrdx__
	CALL_ @strrv
	CALL_ @wrout
	XORrr Rrdi__ Rrdi__
	CALL_ @exit_
:label
	DB___ x0048_
	DB___ x0065_
	DB___ x006c_
	DB___ x006c_
	DB___ x006f_
	DB___ x0020_
	DB___ x0057_
	DB___ x006f_
	DB___ x0072_
	DB___ x006c_
	DB___ x0064_
	DB___ x000a_
:memcp
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrdi__
	PUSH_ Rrsi__
	PUSH_ Rrdx__
:_mcpl
	LRPb_ Rrax__ Rrsi__ x0000_
	SRPb_ Rrdi__ x0000_ Rrax__
	INC__ Rrsi__
	INC__ Rrdi__
	DEC__ Rrdx__
	TSTrr Rrdx__ Rrdx__
	JNE__ @_mcpl
	POP__ Rrdx__
	POP__ Rrsi__
	POP__ Rrdi__
	POP__ Rrbp__
	RET__
:strrv
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrdi__
	PUSH_ Rrsi__
	PUSH_ Rrcx__
	PUSH_ Rrax__
	PUSH_ Rrdx__
	MOVrr Rrcx__ Rrdi__
	ADDrr Rrcx__ Rrsi__
	SUBrw Rrcx__ x0001_
:_srvl
	LRPb_ Rrax__ Rrdi__ x0000_
	LRPb_ Rrdx__ Rrcx__ x0000_
	SRPb_ Rrdi__ x0000_ Rrdx__
	SRPb_ Rrcx__ x0000_ Rrax__
	ADDrw Rrdi__ x0001_
	SUBrw Rrcx__ x0001_
	CMPrr Rrdi__ Rrcx__
	JLT__ @_srvl
	POP__ Rrdx__
	POP__ Rrax__
	POP__ Rrcx__
	POP__ Rrsi__
	POP__ Rrdi__
	POP__ Rrbp__
	RET__
:wrout
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrdi__
	PUSH_ Rrsi__
	LSVq_ Rrsi__ x0008_
	LSVq_ Rrdx__ x0000_
	MOVrw Rrdi__ x0001_
	MOVrw Rrax__ x0001_
	SYSCL
	POP__ Rrsi__
	POP__ Rrdi__
	POP__ Rrbp__
	RET__
:exit_
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrax__
	MOVrw Rrax__ x003c_
	SYSCL
	POP__ Rrax__
	POP__ Rrbp__
	RET__
:mmaps
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrdi__
	PUSH_ Rrsi__
	PUSH_ Rrdx__
	PUSH_ Rrcx__
	PUSH_ Rr8___
	PUSH_ Rr9___
	PUSH_ Rr10__
	MOVrr Rrsi__ Rrdi__
	MOVrw Rrax__ x0009_
	XORrr Rrdi__ Rrdi__
	MOVrw Rrdx__ x0003_
	MOVrw Rr10__ x0022_
	MOVrq Rr8___ xffff_ xffff_ xffff_ xffff_
	XORrr Rr9___ Rr9___
	SYSCL
	POP__ Rr10__
	POP__ Rr9___
	POP__ Rr8___
	POP__ Rrcx__
	POP__ Rrdx__
	POP__ Rrsi__
	POP__ Rrdi__
	POP__ Rrbp__
	RET__

