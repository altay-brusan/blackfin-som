.section boot_code_core_b;


// *********************************************************
// 
// put core b in idle mode just for changing frequencies.
//
// *********************************************************
start_core_b:
	IDLE;
	JUMP.S -2;
start_core_b.end:

// *********************************************************

