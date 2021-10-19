[![Build status](https://ci.appveyor.com/api/projects/status/0y9kmj6esf3uqoqk?svg=true)](https://ci.appveyor.com/project/raspopov/slap)
[![GitHub All Releases](https://img.shields.io/github/downloads/raspopov/SLAP/total)](https://github.com/raspopov/SLAP/releases)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/raspopov/SLAP)](https://github.com/raspopov/SLAP/releases)

 SLAP - Second Life Avatar Probe
---------------------------------

Shows Second Life friends list and notify about their online/offline status.

 Command-line options
----------------------

	-tray - Minimize application to the tray

	-lang:x - Force application language, where x - 2 or 4-digits hexadecimal code of language (see Translations below)

 Hotkeys
---------

    Enter, Left Double Click:
        Avatar Options

    Alt + Left Double Click:
        Open Web-Profile

    Ctrl + Left Double Click:
        Teleport To

    Shift + Left Double Click:
        Simulate notification with current online status

     Shift + Ctrl + Left Double Click:
        Simulate notification with opposite online status

    Delete:
        Delete selected avatar(s)

    F1:
        Open SLAP web-site

    F5:
        Refresh list

    Ctrl + A:
        Select all avatars

	Esc
		Deselect all avatars

    Ctrl + P:
        Global Options

    Ctrl + C, Ctrl + Insert:
        Copy to the clipboard

	Shift + Mouse (move or resize)
		Snap main window to the screen edges

 Translations
--------------

Complete translations and language codes:

	* English (09) by Nikolay Raspopov
	* Russian (19) by Nikolay Raspopov

How to add a new translation:

	1) Download and install poEdit utility - https://poedit.net/
	2) Download language template file: "SLAP.exe.pot" - https://github.com/raspopov/SLAP/blob/master/SLAP/SLAP.exe.pot
	3) Open "SLAP.exe.pot" by poEdit and press "Create new translation" button (at bottom of poEdit window)
	4) Save newly created file under name "SLAP.exe.xx.po" near "SLAP.exe" inside application installation folder,
	   where xx - 2 or 4-digit hexadecimal language code, for example Russian language code is "19" so Russian translation must be
	   named as "SLAP.exe.19.po"
	5) Use "-lang:xx" command-line option to force loading different language, for example to force Russian you must run "SLAP.exe -lang:19"
	6) Send completed translation to <raspopov@cherubicsoft.com>

 Licenses
------------

 Second Life Avatar Probe (SLAP)
---------------------------------

    Copyright (C) 2015-2021 Nikolay Raspopov <raspopov@cherubicsoft.com>

    https://www.cherubicsoft.com/projects/slap/

    This program is free software : you can redistribute it and / or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    ( at your option ) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < http://www.gnu.org/licenses/>.

 WinSparkle
------------

    Copyright (c) 2009-2021 Vaclav Slavik

    https://winsparkle.org

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is furnished to do
    so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 CTrayIcon
-----------

    Copyright (c) Istvan Pasztor

    https://www.codeproject.com/Articles/127791/A-simple-and-easy-to-use-trayicon

    This source has been published on www.codeproject.com under the CPOL license.
