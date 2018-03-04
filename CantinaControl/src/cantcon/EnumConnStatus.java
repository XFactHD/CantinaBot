/*  Copyright (C) <2017>  <XFactHD>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses. */

package cantcon;

//Enumeration of the different states of the serial connection
public enum EnumConnStatus
{
    DISCONNECTED ("Disconnected    ", "#FF0000"),
    CONNECTING   ("Connecting...   ", "#FF8800"),
    CONNECTED    ("Connected       ", "#00FF00"),
    DISCONNECTING("Disconnecting...", "#FF8800");

    private String text;
    private String color;

    EnumConnStatus(String text, String color)
    {
        this.text = text;
        this.color = color;
    }

    //Returns the text to be displayed for a certain state
    public String getText()
    {
        return text;
    }

    //Returns the text color for a certain state
    public String getColor()
    {
        return color;
    }
}