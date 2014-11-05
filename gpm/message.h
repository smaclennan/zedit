/*
 * defines used for messaging
 *
 * Copyright (c) 2001,2002    Nico Schottelius <nico@schottelius.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _MESSAGE_H
#define _MESSAGE_H

/* for print (extern) */
#define GPM_PR_DEBUG  2
#define GPM_PR_INFO   3
#define GPM_PR_ERR    4
#define GPM_PR_WARN   5
#define GPM_PR_OOPS   6

#define GPM_MESS_DOUBLE_S           "%s: %s"
#define GPM_MESS_NO_MEM             "I couln't get any memory! I die! :("
#define GPM_MESS_WRITE_ERR          "write(): %s"
#define GPM_MESS_SOCKET             "socket(): %s"

#endif /* _MESSAGE_H */
