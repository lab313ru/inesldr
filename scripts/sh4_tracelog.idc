
#include <idc.idc>

static main(void) {
	auto logFileName, logFile;

	logFileName = AskFile(0,"*.log","Choose the log-file");

	if(logFileName=="") return -1;

	logFile = fopen(logFileName, "rb");

	if (0 != logFile) {
		auto disaEa, ea, count, optype, opvalue;

		count = 0;

		do {
			disaEa = readstr(logFile);
			ea = xtol(substr(disaEa,0,8));
			MakeUnkn(ea, DOUNK_SIMPLE);
			if(MakeCode(ea) == 0) {
				MakeComm(ea, "Can't Make Code");
			} else {
				optype = GetOpType(ea,0);	
				if( (optype == 2) || (optype == 5)) {	
					opvalue = GetOperandValue(ea,0);
					if(((opvalue & 0x1fffffff) >= 0x0c000000)&&((opvalue & 0x1fffffff) < 0x0d000000))
						OpOff(ea,0,0);
				}
				MakeComm(ea, "code used");
			}
			count++;
		} while (disaEa != -1);
		Message("Log-file has been processed successfully (%d entries).\n", count);
	} else {
		Message("Can't open symbol file:\n%s\n", logFileName);
		return -1;
	}
}
