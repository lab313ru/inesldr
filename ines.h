#ifndef __INES_H__
#define __INES_H__

#define MAX_NAME 16

struct ines_hdr {
	unsigned int signature;
	char	prg_banks;
	char	chr_banks;
	char	map_byte0;
	char	map_byte1;
	char	dummy[8];
};

struct ines_vect {
	unsigned short nmi_vect;
	unsigned short res_vect;
	unsigned short irq_vect;
};

#endif
