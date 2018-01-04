#ifndef __INES_H__
#define __INES_H__

#define MAX_NAME 16

struct ines_hdr {
	unsigned int	signature;
	unsigned char	prg_banks;
	unsigned char	chr_banks;
	unsigned char	map_byte0;
	unsigned char	map_byte1;
	char	dummy[8];
};

struct ines_vect {
	unsigned short nmi_vect;
	unsigned short res_vect;
	unsigned short irq_vect;
};

#endif
