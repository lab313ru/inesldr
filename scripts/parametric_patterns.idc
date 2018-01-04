
#include <idc.idc>
#include <functions.idc>

// SANDBOX

// WC		- local offset (current bank or bank0)
// W0-W3	- direct offset (up to 4 configurable)
// B0-B3	- prg bank for direct offset (up to 4)
// ?		- any char
// *		- any chars
// S		- get next byte as a LEN, then get LEN chars after it as array

static script_convert0() {
	auto ea = ScreenEA();
	auto cnt=0, byt, len;

	len=0;
	while((byt=Byte(ea+len))!=0xFF)
		len++;
	ea = make_data_array(ea,len+1,"");
	Jump(ea);
}

static main(void) {

//	                   opcode,  size, segmask, bankop, ofsop, delta, shift, dobankbyte
//	parametric_farcall("20 C4 EE", 3,    0x3F,      3,     4,     0,     0, 1);
//	parametric_farcall("20 6F D7", 3,       7,      3,     4,     0,     1, 1);
//	parametric_farcall("CD A7 05", 3,    0x3F,      5,     3,     0,     0, 1);
//	parametric_farcall("CD 62 21", 3,    0x3F,      5,     3,     0,     0, 1);

//	parametric_switch("20 5A D0");

	parametric_fixsize("20 70 E2", 2);
// 	parametric_fixsize("20 6B E3", 3);
//	parametric_fixsize("20 4A C4", 2);
//	parametric_fixsize("CD 58 2A", 5);
//	parametric_fixsize("?? E7 04", 4);

//	parametric_stopbytes("20 53 E5", "CF");
//	parametric_stopbytes("CD 96 05", "00");

//	make_8bit_near_tbl(0x21DB7, 0x21D99, -1, 0);

//	make_8bit_far_tbl(0x8000,0x8100,0, 157, 0);
//	make_8bit_far_tbl(0x1852d5,0x18527b, 0x0E, 0, 0);

//	AddHotkey("Shift-M", "script_convert0");
}




