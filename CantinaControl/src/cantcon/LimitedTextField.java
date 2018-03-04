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

import javafx.scene.control.TextField;

//Custom implementation of the TextField with a limited text length
public class LimitedTextField extends TextField
{
    private final int limit;

    public LimitedTextField(int limit)
    {
        this.limit = limit;
    }

    @Override
    public void replaceText(int start, int end, String text)
    {
        super.replaceText(start, end, text);
        verify();
    }

    @Override
    public void replaceSelection(String text)
    {
        super.replaceSelection(text);
        verify();
    }

    private void verify()
    {
        if (getText().length() > limit)
        {
            setText(getText().substring(0, limit));
            positionCaret(limit);
        }
    }
}