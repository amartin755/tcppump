/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2019 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FORMATEXCEPTION_HPP_
#define FORMATEXCEPTION_HPP_

const int exParUnknown = 1;
const int exParRange   = 2;
const int exParFormat  = 3;

class FormatException
{
public:
	FormatException (int cause, const char* val)
	{
		this->cause = cause;
		this->val   = val;
	}

	int what ()
	{
		return cause;
	}

	const char* value ()
	{
		return val;
	}

private:
	int cause;
	const char* val;
};



#endif /* FORMATEXCEPTION_HPP_ */
