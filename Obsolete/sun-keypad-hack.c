#ifdef sun
			/* Hack to get keypad working as a keypad when NumLock is
			 * on.
			 *
			 * Notes: cursor keys will not work will NumLock on
			 *        hitting shift/meta/ctrl turns off NumLock
			 */
			if ((Special & NumLock) == NumLock) {
				switch (key) {
				case 0xff63: c = '0'; key = XK_0; break;
				case 0xffde: c = '1'; key = XK_1; break;
				case 0xff54: c = '2'; key = XK_2; break;
				case 0xffe0: c = '3'; key = XK_3; break;
				case 0xff51: c = '4'; key = XK_4; break;
				case 0xffdc: c = '5'; key = XK_5; break;
				case 0xff53: c = '6'; key = XK_6; break;
				case 0xffd8: c = '7'; key = XK_7; break;
				case 0xff52: c = '8'; key = XK_8; break;
				case 0xffda: c = '9'; key = XK_9; break;

				case 0xffab: c = '+'; key = XK_plus; break;
				case 0xffd5: c = '-'; key = XK_minus; break;
				case 0xffd6: c = '/'; key = XK_slash; break;
				case 0xffd7: c = '*'; key = XK_asterisk; break;
				case 0xffff: c = '.'; key = XK_period; break;
				case 0xff8d: c = '\n'; key = XK_Return; break;
				}
			}

				if (key == 0x1005ff78)
				{	/* Sun5 keyboard sound off/on key */
					Vars[VSILENT].val ^= 1;
					continue;
				}

				/* Sun page keys do not autorepeat */
				if (key == XK_Page_Up || key == XK_Page_Down)
				{
					if (Dragging == PAGING)
						SetTimer(&event, PAGE_TIMEOUT);
					else
					{	/* make first timeout longer */
						SetTimer(&event, PAGE_TIMEOUT * 5);
						Dragging = PAGING;
					}
				}
#endif
