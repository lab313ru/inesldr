
#include <idc.idc>

static main(void) {
	auto i, opcode, offs, reg, eaddr = FirstSeg(), maxaddr = SegEnd(eaddr);
	Message("Segment from %08x to %08x found.\n", eaddr, maxaddr);
	eaddr = FindUnexplored(eaddr, SEARCH_DOWN);
	Message("Unexplored %08x found.\n", eaddr);
	do {
		auto data = Word(eaddr);
		if(((eaddr & 1) == 0) && (data != 0x0000) && (data != 0xFFFF)) {
			Message("Unexplored %08x found, trying to analyse\n", eaddr);
			AutoMark(eaddr, AU_CODE);
			AutoMark(eaddr, AU_PROC);
			Wait();
		} else {
			MakeData(eaddr, FF_BYTE, 1, 0);
		}
		eaddr = FindUnexplored(eaddr, SEARCH_DOWN);
		if(eaddr == BADADDR) {
			eaddr = NextSeg(maxaddr);
			maxaddr = SegEnd(eaddr);
			if(eaddr != BADADDR)
				Message("Segment from %08x to %08x found.\n", eaddr, maxaddr);
		}
	} while (eaddr != BADADDR);
}
