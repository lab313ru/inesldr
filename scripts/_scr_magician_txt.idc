#include <idc.idc>
#include <functions.idc>

// magician text decoder
static magician_txt_uncrunch() {
	auto ea = 0x22000, len, i, j;
	auto eastop = 0x23f86;

	while(ea < eastop) {
	   len = Byte(ea) + 1;
	   MakeUnknown(ea, len, DOUNK_SIMPLE);
	   MakeData(ea, FF_BYTE, len, 0);
	   SetArrayFormat(ea, AP_IDXHEX, len, 0);
  	   ea = ea + len;
	}
}

static main(void) {
//	magician_txt_uncrunch();
}
