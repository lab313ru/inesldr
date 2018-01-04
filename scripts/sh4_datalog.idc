
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
			MakeDword(ea);
			count++;
		} while (disaEa != -1);
		Message("Log-file has been processed successfully (%d entries).\n", count);
	} else {
		Message("Can't open symbol file:\n%s\n", logFileName);
		return -1;
	}
}
