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

//--------------------------------------------------------------------------
//
//      load file into the database.
//
void idaapi load_file(linput_t *li, ushort /*neflag*/, const char * /*fileformatname*/)
{
	int curea, ofs, size;
	unsigned char mapper, _prg;
	sel_t sel = 1;

	if (ph.id != PLFM_6502)
		set_processor_type("M6502", SETPROC_LOADER);

	qlseek(li, 0, SEEK_SET);
	if (qlread(li, &_hdr, sizeof(_hdr)) != sizeof(_hdr)) loader_failure();

	if (!add_segm(0, 0x0000, 0x8000, "RAM", "CODE")) loader_failure();

	mapper = ((_hdr.map_byte0 & 0xF0) >> 4) | (_hdr.map_byte1 & 0xF0);
	_prg = _hdr.prg_banks;

	create_filename_cmt();
	add_pgm_cmt("Mapper      : %d", mapper);
	add_pgm_cmt("16k Banks   : %d", _prg);

	switch (mapper)
	{
	case 0:
	case 3:
	{
		set_selector(sel, 0);
		if (_prg == 1)
		{
			curea = 0xC000;
			size = 0x4000;
		}
		else
		{
			curea = 0x8000;
			size = 0x8000;
		}
		if (!add_segm(sel, curea, curea + size, "ROM", "CODE")) loader_failure();
		file2base(li, qltell(li), curea, curea + size, FILEREG_PATCHABLE);
		break;
	}
	case 1:
	case 2:
	{
		int i;
		curea = 0;
		ofs = 0x8000;
		size = 0x4000;
		for (i = 0; i < _prg - 1; i++)
		{
			set_selector(sel, curea >> 4);
			curea += ofs;
			load_bank(li, curea, size, sel);
			sel++;
			curea += size;
		}
		ofs = 0xC000;
		set_selector(sel, curea >> 4);
		curea += ofs;
		load_bank(li, curea, size, sel);
		sel++;
		curea += size;
		break;
	}
	case 4:
	case 121:
	{
		int i;
		_prg <<= 1;
		curea = 0;
		ofs = 0x8000;
		size = 0x2000;
		for (i = 0; i < _prg - 2; i++)
		{
			set_selector(sel, curea >> 4);
			curea += ofs;
			load_bank(li, curea, size, sel);
			sel++;
			curea += size;
			ofs ^= 0x2000;
		}
		ofs = 0xC000;
		set_selector(sel, curea >> 4);
		curea += ofs;
		load_bank(li, curea, size, sel);
		sel++;
		curea += size;
		ofs = 0xE000;
		set_selector(sel, curea >> 4);
		curea += ofs;
		load_bank(li, curea, size, sel);
		sel++;
		curea += size;
		break;
	}
	case 7:
	case 11:
	case 216:
	default:
	{
		int i;
		_prg >>= 1;
		curea = 0;
		ofs = 0x8000;
		size = 0x8000;
		for (i = 0; i < _prg; i++)
		{
			set_selector(sel, curea >> 4);
			curea += ofs;
			load_bank(li, curea, size, sel);
			sel++;
			curea += size;
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

	add_entry(1, to_ea(sel2para(sel - 1), _vect.nmi_vect), "NMI", true);
	add_entry(2, to_ea(sel2para(sel - 1), _vect.res_vect), "RESET", true);
	add_entry(3, to_ea(sel2para(sel - 1), _vect.irq_vect), "IRQ", true);
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