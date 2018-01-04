/*
 *      Interactive disassembler (IDA)
 *      Copyright (c) 1990-98 by Ilfak Guilfanov.
 *                        E-mail: ig@datarescue.com
 *      iNES loader v0.01
 *      Copyright (c) 2008    by CaH4e3.
 *                        E-mail: cah4e3@gmail.com
 *      ALL RIGHTS RESERVED.
 *
 */

 /*
		 L O A D E R  for iNES files for Famicom/NES emulators
 */

#include <ida.hpp>
#include <loader.hpp>
#include <idp.hpp>
#include <diskio.hpp>
#include <entry.hpp>

#include "ines.h"

static ines_hdr _hdr;
static ines_vect _vect;

//--------------------------------------------------------------------------
//
//      check input file format. if recognized, then return 1
//      and fill 'fileformatname'.
//      otherwise return 0
//
int idaapi accept_file(qstring *fileformatname, qstring *processor, linput_t *li, const char *filename)
{
	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) return(0);
	if (_hdr.signature == 0x1A53454E)
	{
		fileformatname->clear();
		fileformatname->append("Famicom/NES emulator iNES ROM file");
		return(1);
	}
	else
		return(0);
}

//--------------------------------------------------------------------------
void load_bank(linput_t *li, int curea, int size, int sel)
{
	char name[MAX_NAME];
	qsnprintf(name, MAX_NAME, "BANK%01X", sel - 1);
	if (!add_segm(sel, curea, curea + size, name, "CODE")) loader_failure();
	file2base(li, qltell(li), curea, curea + size, FILEREG_PATCHABLE);
}

static unsigned char *_cdl = NULL;
static FILE *cdl_file = NULL;

//--------------------------------------------------------------------------
bool cdl_load() {
	char f_name[1024];
	int f_name_size;
	uint64 f_size;
	get_input_file_path(f_name, sizeof(f_name));
	f_name_size = strlen(f_name);
	f_name[f_name_size - 1] = 'l';
	f_name[f_name_size - 2] = 'd';
	f_name[f_name_size - 3] = 'c';
	if ((cdl_file = qfopen(f_name, "rb")) != NULL) {
		add_pgm_cmt("CDL file    : present, processing");
		f_size = qfsize(cdl_file);
		_cdl = (unsigned char*)malloc(f_size);
		qfread(cdl_file, _cdl, f_size);
		qfclose(cdl_file);
		return 1;
	}
	else
		return 0;
}

int cdl_get_bank_org(int n_bank, int s_bank, int d_bank) {
	int seg[5] = { 0, 0, 0, 0, 0 }, start = n_bank * s_bank, stop = start + s_bank, i, res_n = 0, res_i = 0;
	if (_cdl != NULL) {
		for (i = start; i < stop; i++)
			if (_cdl[i] & 1) {
				if (_cdl[i] & 0x80)
					seg[0]++;
				else
					seg[((_cdl[i] >> 2) & 3) + 1]++;
			}
		if (seg[0] | seg[1] | seg[2] | seg[3] | seg[4]) {
			res_n = seg[0];
			res_i = 0;
			if (res_n < seg[1]) {
				res_n = seg[1];
				res_i = 1;
			}
			if (res_n < seg[2]) {
				res_n = seg[2];
				res_i = 2;
			}
			if (res_n < seg[3]) {
				res_n = seg[3];
				res_i = 3;
			}
			if (res_n < seg[4]) {
				res_n = seg[4];
				res_i = 4;
			}
			return (0x6000 + (res_i << 13)) & ~(s_bank - 1);
		}
	}
	return d_bank;
}

void cdl_free() {
	if (_cdl != NULL) {
		free(_cdl);
		_cdl = NULL;
	}
}

//--------------------------------------------------------------------------
//
//      load file into the database.
//
void idaapi load_file(linput_t *li, ushort /*neflag*/, const char * /*fileformatname*/)
{
	unsigned int curea, ofs, size, i;
	unsigned int mapper, _prg;
	sel_t sel = 1;

	if (ph.id != PLFM_6502)
		set_processor_type("M6502", SETPROC_LOADER);

	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) loader_failure();

	if (!add_segm(0, 0x0000, 0x8000, "RAM", "CODE")) loader_failure();

	mapper = ((_hdr.map_byte0 & 0xF0) >> 4) | (_hdr.map_byte1 & 0xF0);
	_prg = _hdr.prg_banks;
	if (_prg == 0)
		_prg = 256;

	cdl_load();
	create_filename_cmt();
	add_pgm_cmt("Mapper      : %d", mapper);
	add_pgm_cmt("16k Banks   : %d", _prg);

	curea = 0;
	if (_prg == 1) {
		size = 0x4000;
		set_selector(sel, curea >> 4);
		curea = cdl_get_bank_org(0, size, 0xC000);
		if (!add_segm(sel, curea, curea + size, "ROM", "CODE")) loader_failure();
		file2base(li, qltell(li), curea, curea + size, FILEREG_PATCHABLE);
	}
	else if (_prg == 2) {
		size = 0x8000;
		set_selector(sel, curea >> 4);
		curea = 0x8000;
		if (!add_segm(sel, curea, curea + size, "ROM", "CODE")) loader_failure();
		file2base(li, qltell(li), curea, curea + size, FILEREG_PATCHABLE);
	}
	else {
		switch (mapper) {
		case 1:
		case 2:
		case 10:
		case 16:
		case 68:
		case 71:
		case 153:
		case 156:
		case 157:
		case 159:
		case 182: {
			size = 0x4000;
			for (i = 0; i < _prg - 1; i++)
			{
				set_selector(sel, curea >> 4);
				curea += cdl_get_bank_org(i, size, 0x8000);
				load_bank(li, curea, size, sel);
				sel++;
				curea += 0x10000;
			}
			set_selector(sel, curea >> 4);
			curea += 0xC000;
			load_bank(li, curea, size, sel);
			break;
		}
		case 4:
		case 5:
		case 6:
		case 9:
		case 12:
		case 14:
		case 15:
		case 23:
		case 83:
		case 121: {
			_prg <<= 1;
			ofs = 0x8000;
			size = 0x2000;
			for (i = 0; i < _prg - 2; i++)
			{
				set_selector(sel, curea >> 4);
				curea += cdl_get_bank_org(i, size, ofs);
				load_bank(li, curea, size, sel);
				sel++;
				curea += 0x10000;
				ofs ^= 0x2000;
			}
			set_selector(sel, curea >> 4);
			curea += cdl_get_bank_org(i, size, 0xC000);
			load_bank(li, curea, size, sel);
			sel++;
			curea += 0x10000;
			set_selector(sel, curea >> 4);
			curea += 0xE000;
			load_bank(li, curea, size, sel);
			break;
		}
		case 42: {
			_prg <<= 1;
			size = 0x2000;
			for (i = 0; i < _prg - 4; i++)
			{
				set_selector(sel, curea >> 4);
				curea += 0x6000;
				load_bank(li, curea, size, sel);
				sel++;
				curea += 0x10000;
			}
			size = 0x8000;
			set_selector(sel, curea >> 4);
			curea += 0x8000;
			load_bank(li, curea, size, sel);
			break;
		}
		case 17:
		case 18:
		case 19:
		case 80:
		case 82:
		default: {
			_prg <<= 1;
			size = 0x2000;
			for (i = 0; i < _prg - 1; i++)
			{
				set_selector(sel, curea >> 4);
				curea += cdl_get_bank_org(i, size, 0x8000);
				load_bank(li, curea, size, sel);
				sel++;
				curea += 0x10000;
			}
			set_selector(sel, curea >> 4);
			curea += 0xE000;
			load_bank(li, curea, size, sel);
			break;
		}
		case 7:
		case 11:
		case 13:
		case 216: {
			_prg >>= 1;
			ofs = 0x8000;
			size = 0x8000;
			for (i = 0; i < _prg; i++)
			{
				set_selector(sel, curea >> 4);
				curea += ofs;
				load_bank(li, curea, size, sel);
				sel++;
				curea += 0x10000;
			}
		}
		}
	}

	qlseek(li, (_hdr.prg_banks << 14) + sizeof(_hdr) - sizeof(_vect), SEEK_SET);
	if (qlread(li, &_vect, sizeof(_vect)) != sizeof(_vect)) loader_failure();

	add_pgm_cmt("NMI vector  : $%04X", _vect.nmi_vect);
	add_pgm_cmt("RESET vector: $%04X", _vect.res_vect);
	add_pgm_cmt("IRQ vector  : $%04X", _vect.irq_vect);

	inf.af =
		AF_FIXUP | //   0x0001          // Create offsets and segments using fixup info
		AF_MARKCODE | //   0x0002          // Mark typical code sequences as code
		AF_UNK | //   0x0004          // Delete instructions with no xrefs
		AF_CODE | //   0x0008          // Trace execution flow
		AF_PROC | //   0x0010          // Create functions if call is present
		AF_USED | //   0x0020          // Analyze and create all xrefs
		AF_FLIRT | //   0x0040          // Use flirt signatures
		AF_PROCPTR | //   0x0080          // Create function if data xref data->code32 exists
		AF_NULLSUB | //   0x0200          // Rename empty functions as nullsub_...
		AF_IMMOFF | //   0x2000          // Convert 32bit instruction operand to offset
		AF_DREFOFF; //   0x4000          // Create offset if data xref to seg32 exists
	inf.af2 = 0;

	//  set_default_dataseg(0);

	add_entry(1, to_ea(sel2para(sel), _vect.nmi_vect), "NMI", true);
	add_entry(2, to_ea(sel2para(sel), _vect.res_vect), "RESET", true);
	add_entry(3, to_ea(sel2para(sel), _vect.irq_vect), "IRQ", true);

	cdl_free();
}

//----------------------------------------------------------------------
//
//      LOADER DESCRIPTION BLOCK
//
//----------------------------------------------------------------------
loader_t LDSC =
{
	IDP_INTERFACE_VERSION,
	0, // loader flags
	//      check input file format. if recognized, then return 1
	//      and fill 'fileformatname'.
	//      otherwise return 0
	accept_file,
	//
	// load file into the database.
	//
	load_file,
	//
	//	create output file from the database.
	//	this function may be absent.
	//
	NULL,
	//      take care of a moved segment (fix up relocations, for example)
	NULL,
};