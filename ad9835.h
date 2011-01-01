/***************************************************************************
 *   Copyright (C) 2011 by Ben Nahill                                      *
 *   bnahill@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __AD9535_H_
#define __AD9535_H_

/*
 * The following are not provided since C30 doesn't include stdint.h.
 * If it turns out that other components may need this, a new header should
 * be created to provide this in common.
 */
typedef unsigned int uint16_t; 
typedef unsigned char uint8_t; 
typedef unsigned long uint32_t;

typedef union {
	uint32_t integer;
	struct {
		uint8_t msb_h;
		uint8_t msb_l;
		uint8_t lsb_h;
		uint8_t lsb_l;
	} bytes;
} frequency;

typedef enum { AD_OFF = 0, AD_ON = (1 << 13) } ad9835_state;

void ad9835_init( void );
void ad9835_set_state( ad9835_state state );
void ad9835_set_frequency( frequency freq );

#endif

