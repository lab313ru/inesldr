
#include <idc.idc>
#include <functions.idc>

// MISC SCRIPT CONVERTERS

static script_convert0(void) {
	auto ptr_ofs = 0xB0104;
	auto ptr_pos = 0;
	auto scr_ofs = 0xC397C;
	auto scr_pos = 0;

	Message("Start %08x, %08x\n", ptr_ofs, scr_ofs);

	do {
		auto ptr = 0xB0000 + (Word(ptr_ofs + (ptr_pos << 1)) & 0x3FFF);
		auto idx = 0;
		Message(" Next ptr %04x\n", ptr);

		do {
			auto ofs = ptr + idx;
			auto cmd = Byte(ofs);
			auto siz = Byte(scr_ofs + cmd);
			Message("  CMD %02x, Size %d\n", cmd, siz);
			MakeUnknown(ofs, siz, DOUNK_SIMPLE);
			MakeData(ofs, FF_BYTE, siz, 0);
			idx = idx + siz;
		} while ((cmd != 0x0A)&&(cmd != 0x0B)&&(cmd != 0x0C)&&(cmd != 0x0D)&&(cmd != 0x1B));
		ptr_pos++;
	} while (ptr_pos < 0x8A);

	Message("Done\n");
}

static script_convert1(void) {
	auto str_ofs = 0x80000;
	auto str_cnt = 0;

	do {
		auto str_len = Byte(str_ofs);
		MakeUnknown(str_ofs, str_len, DOUNK_SIMPLE);
		MakeData(str_ofs, FF_BYTE, 2, 0);
		MakeData(str_ofs + 2, FF_BYTE, str_len - 2, 0);
		SetArrayFormat(str_ofs, AP_IDXHEX, 2, 0);
		SetArrayFormat(str_ofs + 2, AP_IDXHEX, str_len - 2, 0);
		auto name = sprintf("_msg%02X",str_cnt);
		MakeName(str_ofs,name);
		str_ofs = str_ofs + str_len;
		str_cnt++;
	} while (str_cnt < 0x117);

	Message("Done\n");
}

static auto_array_FF() {
	auto ea = ScreenEA();
	auto i = 0;
	while(Byte(ea+i)!=0xFF)i++;
	i++;
	MakeUnknown(ea,i,DOUNK_SIMPLE);
	if(MakeData(ea, FF_BYTE, i, 0)) {
		if(SetArrayFormat(ea, AP_IDXHEX, i, 0))
			Jump(ea+i);
		else
			Message("can't set array format!\n");

	} else
		Message("can't make data!\n");
}

static get_cmd_len(cmd) {
	auto ret = -1;
//    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
//0   2,  0,  2,  3,  3,  3,  4,  3,  0,  2,  0,  2,  2,  3,  3,  0 	
//1   0,  2,  2,  2,  1,  2,  2, $D, $D,  2,  2,  2,  4,  3,  0,  0
//2   7, $A,  5,  2,  4,  1,  4,  4,  3,  1,  4,  3,  1,  1,  0,  2
//3   3,  5,  0,  2,  1,  1,  3,  5,  1,  1, $A,  1,  1,  2,  2,  3
//4   4,$40,  2,  4, $E, $E,  1										
	if(cmd==1)			ret=0;
	else if(cmd==2)		ret=2;
	else if(cmd==3)		ret=3;
	else if(cmd==4)		ret=3;
	else if(cmd==5)		ret=3;
	else if(cmd==6)		ret=4;
	else if(cmd==7)		ret=3;
	else if(cmd==8)		ret=0;
	else if(cmd==9)		ret=2;
	else if(cmd==0xA)	ret=0;
	else if(cmd==0xB)	ret=2;
	else if(cmd==0xC)	ret=2;
	else if(cmd==0xD)	ret=3;
	else if(cmd==0xE)	ret=3;
	else if(cmd==0xF)	ret=0;
	else if(cmd==0x10)	ret=0;
	else if(cmd==0x11)	ret=2;
	else if(cmd==0x12)	ret=2;
	else if(cmd==0x13)	ret=2;
	else if(cmd==0x14)	ret=1;
	else if(cmd==0x15)	ret=2;
	else if(cmd==0x16)	ret=2;
	else if(cmd==0x17)	ret=0xD;
	else if(cmd==0x18)	ret=0xD;
	else if(cmd==0x19)	ret=2;
	else if(cmd==0x1A)	ret=2;
	else if(cmd==0x1B)	ret=2;
	else if(cmd==0x1C)	ret=4;
	else if(cmd==0x1D)	ret=3;
	else if(cmd==0x1E)	ret=0;
	else if(cmd==0x1F)	ret=0;
	else if(cmd==0x20)	ret=7;
	else if(cmd==0x21)	ret=0xA;
	else if(cmd==0x22)	ret=5;
	else if(cmd==0x23)	ret=2;
	else if(cmd==0x24)	ret=4;
	else if(cmd==0x25)	ret=1;
	else if(cmd==0x26)	ret=4;
	else if(cmd==0x27)	ret=4;
	else if(cmd==0x28)	ret=3;
	else if(cmd==0x29)	ret=1;
	else if(cmd==0x2A)	ret=4;
	else if(cmd==0x2B)	ret=3;
	else if(cmd==0x2C)	ret=1;
	else if(cmd==0x2D)	ret=1;
	else if(cmd==0x2E)	ret=0;
	else if(cmd==0x2F)	ret=2;
	else if(cmd==0x30)	ret=3;
	else if(cmd==0x31)	ret=5;
	else if(cmd==0x32)	ret=0;
	else if(cmd==0x33)	ret=2;
	else if(cmd==0x34)	ret=1;
	else if(cmd==0x35)	ret=1;
	else if(cmd==0x36)	ret=3;
	else if(cmd==0x37)	ret=5;
	else if(cmd==0x38)	ret=1;
	else if(cmd==0x39)	ret=1;
	else if(cmd==0x3A)	ret=0xA;
	else if(cmd==0x3B)	ret=1;
	else if(cmd==0x3C)	ret=1;
	else if(cmd==0x3D)	ret=1;
	else if(cmd==0x3E)	ret=0;
	else if(cmd==0x3F)	ret=3;
	else if(cmd==0x40)	ret=4;
	else if(cmd==0x41)	ret=0x40;
	else if(cmd==0x42)	ret=2;
	else if(cmd==0x43)	ret=4;
	else if(cmd==0x44)	ret=0xE;
	else if(cmd==0x45)	ret=0xE;
	else if(cmd==0x46)	ret=1;
	return ret;
}

static auto_res_parse() {
	auto ea = ScreenEA();
	auto cmd=Byte(ea);
	auto tmp0, len, i;
	
//	if(cmd==0) {
		tmp0 = Byte(ea+1);
		if(tmp0!=0xFD)
			ea = make_data_array(ea, 2, form("resourse 0x%02X",tmp0));
		else
			make_data_array(ea, 3, form("resourse 0x%02X",tmp0));
//	} else {
		while((cmd=Byte(ea))!=0) {
			len = get_cmd_len(cmd);
//			Message("cmd = %02X, len %02X\n",cmd,len);
			if(len == 0) {
				i=1;
				do {
					tmp0 = Byte(ea+i);
					i++;
				} while((tmp0!=0xFF)&&(tmp0!=0xFE));
				ea = make_data_array(ea, i, "");
			} else if (len == 0x40) {
				i=1;
				while(Byte(ea+i)!=0xFF)i++;
				ea = make_data_array(ea, i+1, "");
				i=1;
				while(Byte(ea+i)!=0xFF)i++;
				ea = make_data_array(ea, i+1, "");
				i=1;
				while(Byte(ea+i)!=0xFF)i++;
				ea = make_data_array(ea, i+1, "");
				i=1;
				while(Byte(ea+i)!=0xFF)i++;
				ea = make_data_array(ea, i+1, "");
				ea = make_data_array(ea, 3, "");
			} else
				ea = make_data_array(ea, len, "");
		}
//	}
	Jump(ea);
}


static main(void) {
	AddHotkey("Shift-0", "auto_array_FF");
	AddHotkey("Shift-9", "auto_res_parse");
}




