
static cdl_script_execute() {
	auto err, cdl_nes = SegByName(form("ROM%X",0))==BADADDR;
	Message("CDL autorun: ");
	if(cdl_nes) {
		Message("NES\n");
		if(CompileEx("G:\\ida\\idc\\cdl_nes.idc",1) == 0)
			main();
		else if(CompileEx("H:\\ida\\idc\\cdl_nes.idc",1) == 0)
			main();
		else if(CompileEx("F:\\ida\\idc\\cdl_nes.idc",1) == 0)
			main();
		else if(CompileEx("C:\\ida\\idc\\cdl_gb.idc",1) == 0)
			main();
		else
			Message("Compilation error: \"%s\"\n",err);
	} else {
		Message("GB\n");
		if(CompileEx("G:\\ida\\idc\\cdl_gb.idc",1) == 0)
			main();
		else if(CompileEx("H:\\ida\\idc\\cdl_gb.idc",1) == 0)
			main();
		else if(CompileEx("F:\\ida\\idc\\cdl_gb.idc",1) == 0)
			main();
		else if(CompileEx("C:\\ida\\idc\\cdl_gb.idc",1) == 0)
			main();
		else
			Message("Compilation error: \"%s\"\n",err);
	}
}
