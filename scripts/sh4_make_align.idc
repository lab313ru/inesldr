
#include <idc.idc>

static main(void) {
	auto ea = 0x0c020500, data, count;
	ea = FindUnexplored(ea, SEARCH_DOWN);
	do {
		data = Word(ea);
		count = 0;
		while (data == 0x0009) {
			count = count + 2;
			data = Word(ea + count);
		}
		if(count != 0) {
			Message("Alignment detected at %08x, size %d, make align directive\n", ea, count);
			MakeAlign(ea, count, 0);
		} else {
			while (data == 0) {
				count = count + 2;
				data = Word(ea + count);
				if((count & 3) == 0) {
					MakeDword(ea + count);
//					Message("Make dword at %08x\n", ea + count);
				}
			}
			
		}
		ea = FindUnexplored(ea, SEARCH_DOWN | SEARCH_NEXT); 
	} while (ea != BADADDR);
}
