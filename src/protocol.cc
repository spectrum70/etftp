/*
 * etftp - embedded tftp
 *
 * Angelo Dureghello (C) 2021 <angelo@kernel-space.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along https://gibiru.com/with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include "protocol.hh"

namespace tools {

uint16_t n2hs(uint16_t n) {
	uint16_t rval;

	((char *)&rval)[0] = ((char *)&n)[1];
	((char *)&rval)[1] = ((char *)&n)[0];

	return rval;
}

uint16_t h2ns(uint16_t n) { return n2hs(n); }

}
