	MOVrw Rrdi__ x1000_
	CALL_ @mmaps
	PUSH_ Rrax__
	MOVrr Rrdi__ Rrax__
	MOVrw Rrsi__ x1000_
	CALL_ @rdin_
	MOVrr Rrsi__ Rrax__
	POP__ Rrdi__
	CALL_ @wrout
	XORrr Rrdi__ Rrdi__
	CALL_ @exit_
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
:rdin_
	PUSH_ Rrbp__
	MOVrr Rrbp__ Rrsp__
	PUSH_ Rrdi__
	PUSH_ Rrsi__
	PUSH_ Rrdx__
	XORrr Rrax__ Rrax__
	XORrr Rrdi__ Rrdi__
	LSVq_ Rrsi__ x0010_
	LSVq_ Rrdx__ x0008_
	SYSCL
	POP__ Rrdx__
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

