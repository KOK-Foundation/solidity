/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file CommonData.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/Assertions.h>
#include <libdevcore/Keccak256.h>
#include <libdevcore/FixedHash.h>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace dev;

namespace
{

static char const* upperHexChars = "0123456789ABCDEF";
static char const* lowerHexChars = "0123456789abcdef";

}

string dev::toHex(uint8_t _data, HexCase _case)
{
	assertThrow(_case != HexCase::Mixed, BadHexCase, "Mixed case can only be used for byte arrays.");

	char const* chars = _case == HexCase::Upper ? upperHexChars : lowerHexChars;

	return std::string{
		chars[(unsigned(_data) / 16) & 0xf],
		chars[unsigned(_data) & 0xf]
	};
}

string dev::toHex(bytes const& _data, HexPrefix _prefix, HexCase _case)
{
	std::string ret(_data.size() * 2 + (_prefix == HexPrefix::Add ? 2 : 0), 0);

	size_t i = 0;
	if (_prefix == HexPrefix::Add)
	{
		ret[i++] = '0';
		ret[i++] = 'x';
	}

	// Mixed case will be handled inside the loop.
	char const* chars = _case == HexCase::Upper ? upperHexChars : lowerHexChars;
	int rix = _data.size() - 1;
	for (uint8_t c: _data)
	{
		// switch hex case every four hexchars
		if (_case == HexCase::Mixed)
			chars = (rix-- & 2) == 0 ? lowerHexChars : upperHexChars;

		ret[i++] = chars[(unsigned(c) / 16) & 0xf];
		ret[i++] = chars[unsigned(c) & 0xf];
	}
	assertThrow(i == ret.size(), Exception, "");

	return ret;
}

int dev::fromHex(char _i, WhenError _throw)
{
	if (_i >= '0' && _i <= '9')
		return _i - '0';
	if (_i >= 'a' && _i <= 'f')
		return _i - 'a' + 10;
	if (_i >= 'A' && _i <= 'F')
		return _i - 'A' + 10;
	if (_throw == WhenError::Throw)
		assertThrow(false, BadHexCharacter, to_string(_i));
	else
		return -1;
}

bytes dev::fromHex(std::string const& _s, WhenError _throw)
{
	unsigned s = (_s.size() >= 2 && _s[0] == '0' && _s[1] == 'x') ? 2 : 0;
	std::vector<uint8_t> ret;
	ret.reserve((_s.size() - s + 1) / 2);

	if (_s.size() % 2)
	{
		int h = fromHex(_s[s++], _throw);
		if (h != -1)
			ret.push_back(h);
		else
			return bytes();
	}
	for (unsigned i = s; i < _s.size(); i += 2)
	{
		int h = fromHex(_s[i], _throw);
		int l = fromHex(_s[i + 1], _throw);
		if (h != -1 && l != -1)
			ret.push_back((uint8_t)(h * 16 + l));
		else
			return bytes();
	}
	return ret;
}


bool dev::passesAddressChecksum(string const& _str, bool _strict)
{
	string s = _str.substr(0, 2) == "0x" ? _str : "0x" + _str;

	if (s.length() != 42)
		return false;

	if (!_strict && (
		s.find_first_of("abcdef") == string::npos ||
		s.find_first_of("ABCDEF") == string::npos
	))
		return true;

	return s == dev::getChecksummedAddress(s);
}

string dev::getChecksummedAddress(string const& _addr)
{
	string s = _addr.substr(0, 2) == "0x" ? _addr.substr(2) : _addr;
	assertThrow(s.length() == 40, InvalidAddress, "");
	assertThrow(s.find_first_not_of("0123456789abcdefABCDEF") == string::npos, InvalidAddress, "");

	h256 hash = keccak256(boost::algorithm::to_lower_copy(s, std::locale::classic()));

	string ret = "0x";
	for (size_t i = 0; i < 40; ++i)
	{
		char addressCharacter = s[i];
		unsigned nibble = (unsigned(hash[i / 2]) >> (4 * (1 - (i % 2)))) & 0xf;
		if (nibble >= 8)
			ret += toupper(addressCharacter);
		else
			ret += tolower(addressCharacter);
	}
	return ret;
}

bool dev::isValidHex(string const& _string)
{
	if (_string.substr(0, 2) != "0x")
		return false;
	if (_string.find_first_not_of("0123456789abcdefABCDEF", 2) != string::npos)
		return false;
	return true;
}

bool dev::isValidDecimal(string const& _string)
{
	if (_string.empty())
		return false;
	if (_string == "0")
		return true;
	// No leading zeros
	if (_string.front() == '0')
		return false;
	if (_string.find_first_not_of("0123456789") != string::npos)
		return false;
	return true;
}

// Returns a quoted string if all characters are printable ASCII chars,
// or its hex representation otherwise.
std::string dev::formatAsStringOrNumber(std::string const& _value)
{
	if (_value.length() <= 32)
		for (auto const& c: _value)
			if (c <= 0x1f || c >= 0x7f || c == '"')
				return "0x" + h256(_value, h256::AlignLeft).hex();

	return "\"" + _value + "\"";
}
