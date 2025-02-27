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
/** @file CommonData.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Shared algorithms and data types.
 */

#pragma once

#include <libdevcore/Common.h>

#include <vector>
#include <type_traits>
#include <cstring>
#include <optional>
#include <string>
#include <set>
#include <functional>
#include <utility>

/// Operators need to stay in the global namespace.

/// Concatenate the contents of a container onto a vector
template <class T, class U> std::vector<T>& operator+=(std::vector<T>& _a, U const& _b)
{
	for (auto const& i: _b)
		_a.push_back(i);
	return _a;
}
/// Concatenate the contents of a container onto a vector, move variant.
template <class T, class U> std::vector<T>& operator+=(std::vector<T>& _a, U&& _b)
{
	std::move(_b.begin(), _b.end(), std::back_inserter(_a));
	return _a;
}
/// Concatenate the contents of a container onto a set
template <class T, class U> std::set<T>& operator+=(std::set<T>& _a, U const& _b)
{
	_a.insert(_b.begin(), _b.end());
	return _a;
}
/// Concatenate the contents of a container onto a set, move variant.
template <class T, class U> std::set<T>& operator+=(std::set<T>& _a, U&& _b)
{
	for (auto&& x: _b)
		_a.insert(std::move(x));
	return _a;
}
/// Concatenate two vectors of elements.
template <class T>
inline std::vector<T> operator+(std::vector<T> const& _a, std::vector<T> const& _b)
{
	std::vector<T> ret(_a);
	ret += _b;
	return ret;
}
/// Concatenate two vectors of elements, moving them.
template <class T>
inline std::vector<T> operator+(std::vector<T>&& _a, std::vector<T>&& _b)
{
	std::vector<T> ret(std::move(_a));
	if (&_a == &_b)
		ret += ret;
	else
		ret += std::move(_b);
	return ret;
}
/// Concatenate something to a sets of elements.
template <class T, class U>
inline std::set<T> operator+(std::set<T> const& _a, U&& _b)
{
	std::set<T> ret(_a);
	ret += std::forward<U>(_b);
	return ret;
}
/// Concatenate something to a sets of elements, move variant.
template <class T, class U>
inline std::set<T> operator+(std::set<T>&& _a, U&& _b)
{
	std::set<T> ret(std::move(_a));
	ret += std::forward<U>(_b);
	return ret;
}
/// Remove one set from another one.
template <class T>
inline std::set<T>& operator-=(std::set<T>& _a, std::set<T> const& _b)
{
	for (auto const& x: _b)
		_a.erase(x);
	return _a;
}

namespace dev
{

// String conversion functions, mainly to/from hex/nibble/byte representations.

enum class WhenError
{
	DontThrow = 0,
	Throw = 1,
};

enum class HexPrefix
{
	DontAdd = 0,
	Add = 1,
};

enum class HexCase
{
	Lower = 0,
	Upper = 1,
	Mixed = 2,
};

/// Convert a single byte to a string of hex characters (of length two),
/// optionally with uppercase hex letters.
std::string toHex(uint8_t _data, HexCase _case = HexCase::Lower);

/// Convert a series of bytes to the corresponding string of hex duplets,
/// optionally with "0x" prefix and with uppercase hex letters.
std::string toHex(bytes const& _data, HexPrefix _prefix = HexPrefix::DontAdd, HexCase _case = HexCase::Lower);

/// Converts a (printable) ASCII hex character into the corresponding integer value.
/// @example fromHex('A') == 10 && fromHex('f') == 15 && fromHex('5') == 5
int fromHex(char _i, WhenError _throw);

/// Converts a (printable) ASCII hex string into the corresponding byte stream.
/// @example fromHex("41626261") == asBytes("Abba")
/// If _throw = ThrowType::DontThrow, it replaces bad hex characters with 0's, otherwise it will throw an exception.
bytes fromHex(std::string const& _s, WhenError _throw = WhenError::DontThrow);
/// Converts byte array to a string containing the same (binary) data. Unless
/// the byte array happens to contain ASCII data, this won't be printable.
inline std::string asString(bytes const& _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}

/// Converts byte array ref to a string containing the same (binary) data. Unless
/// the byte array happens to contain ASCII data, this won't be printable.
inline std::string asString(bytesConstRef _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}

/// Converts a string to a byte array containing the string's (byte) data.
inline bytes asBytes(std::string const& _b)
{
	return bytes((uint8_t const*)_b.data(), (uint8_t const*)(_b.data() + _b.size()));
}

// Big-endian to/from host endian conversion functions.

/// Converts a templated integer value to the big-endian byte-stream represented on a templated collection.
/// The size of the collection object will be unchanged. If it is too small, it will not represent the
/// value properly, if too big then the additional elements will be zeroed out.
/// @a Out will typically be either std::string or bytes.
/// @a T will typically by unsigned, u160, u256 or bigint.
template <class T, class Out>
inline void toBigEndian(T _val, Out& o_out)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	for (auto i = o_out.size(); i != 0; _val >>= 8, i--)
	{
		T v = _val & (T)0xff;
		o_out[i - 1] = (typename Out::value_type)(uint8_t)v;
	}
}

/// Converts a big-endian byte-stream represented on a templated collection to a templated integer value.
/// @a _In will typically be either std::string or bytes.
/// @a T will typically by unsigned, u160, u256 or bigint.
template <class T, class _In>
inline T fromBigEndian(_In const& _bytes)
{
	T ret = (T)0;
	for (auto i: _bytes)
		ret = (T)((ret << 8) | (uint8_t)(typename std::make_unsigned<typename _In::value_type>::type)i);
	return ret;
}
inline bytes toBigEndian(u256 _val) { bytes ret(32); toBigEndian(_val, ret); return ret; }
inline bytes toBigEndian(u160 _val) { bytes ret(20); toBigEndian(_val, ret); return ret; }

/// Convenience function for toBigEndian.
/// @returns a byte array just big enough to represent @a _val.
template <class T>
inline bytes toCompactBigEndian(T _val, unsigned _min = 0)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	int i = 0;
	for (T v = _val; v; ++i, v >>= 8) {}
	bytes ret(std::max<unsigned>(_min, i), 0);
	toBigEndian(_val, ret);
	return ret;
}

/// Convenience function for conversion of a u256 to hex
inline std::string toHex(u256 val, HexPrefix prefix = HexPrefix::DontAdd)
{
	std::string str = toHex(toBigEndian(val));
	return (prefix == HexPrefix::Add) ? "0x" + str : str;
}

inline std::string toCompactHexWithPrefix(u256 const& _value)
{
	return toHex(toCompactBigEndian(_value, 1), HexPrefix::Add);
}

/// Returns decimal representation for small numbers and hex for large numbers.
inline std::string formatNumber(bigint const& _value)
{
	if (_value < 0)
		return "-" + formatNumber(-_value);
	if (_value > 0x1000000)
		return toHex(toCompactBigEndian(_value, 1), HexPrefix::Add);
	else
		return _value.str();
}

inline std::string formatNumber(u256 const& _value)
{
	if (_value > 0x1000000)
		return toCompactHexWithPrefix(_value);
	else
		return _value.str();
}


// Algorithms for string and string-like collections.

/// Determine bytes required to encode the given integer value. @returns 0 if @a _i is zero.
template <class T>
inline unsigned bytesRequired(T _i)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
	unsigned i = 0;
	for (; _i != 0; ++i, _i >>= 8) {}
	return i;
}
template <class T, class V>
bool contains(T const& _t, V const& _v)
{
	return std::end(_t) != std::find(std::begin(_t), std::end(_t), _v);
}

/// Function that iterates over a vector, calling a function on each of its
/// elements. If that function returns a vector, the element is replaced by
/// the returned vector. During the iteration, the original vector is only valid
/// on the current element and after that. The actual replacement takes
/// place at the end, but already visited elements might be invalidated.
/// If nothing is replaced, no copy is performed.
template <typename T, typename F>
void iterateReplacing(std::vector<T>& _vector, F const& _f)
{
	// Concept: _f must be Callable, must accept param T&, must return optional<vector<T>>
	bool useModified = false;
	std::vector<T> modifiedVector;
	for (size_t i = 0; i < _vector.size(); ++i)
	{
		if (std::optional<std::vector<T>> r = _f(_vector[i]))
		{
			if (!useModified)
			{
				std::move(_vector.begin(), _vector.begin() + i, back_inserter(modifiedVector));
				useModified = true;
			}
			modifiedVector += std::move(*r);
		}
		else if (useModified)
			modifiedVector.emplace_back(std::move(_vector[i]));
	}
	if (useModified)
		_vector = std::move(modifiedVector);
}

namespace detail
{
template <typename T, typename F, std::size_t... I>
void iterateReplacingWindow(std::vector<T>& _vector, F const& _f, std::index_sequence<I...>)
{
	// Concept: _f must be Callable, must accept sizeof...(I) parameters of type T&, must return optional<vector<T>>
	bool useModified = false;
	std::vector<T> modifiedVector;
	size_t i = 0;
	for (; i + sizeof...(I) <= _vector.size(); ++i)
	{
		if (std::optional<std::vector<T>> r = _f(_vector[i + I]...))
		{
			if (!useModified)
			{
				std::move(_vector.begin(), _vector.begin() + i, back_inserter(modifiedVector));
				useModified = true;
			}
			modifiedVector += std::move(*r);
			i += sizeof...(I) - 1;
		}
		else if (useModified)
			modifiedVector.emplace_back(std::move(_vector[i]));
	}
	if (useModified)
	{
		for (; i < _vector.size(); ++i)
			modifiedVector.emplace_back(std::move(_vector[i]));
		_vector = std::move(modifiedVector);
	}
}

}

/// Function that iterates over the vector @param _vector,
/// calling the function @param _f on sequences of @tparam N of its
/// elements. If @param _f returns a vector, these elements are replaced by
/// the returned vector and the iteration continues with the next @tparam N elements.
/// If the function does not return a vector, the iteration continues with an overlapping
/// sequence of @tparam N elements that starts with the second element of the previous
/// iteration.
/// During the iteration, the original vector is only valid
/// on the current element and after that. The actual replacement takes
/// place at the end, but already visited elements might be invalidated.
/// If nothing is replaced, no copy is performed.
template <std::size_t N, typename T, typename F>
void iterateReplacingWindow(std::vector<T>& _vector, F const& _f)
{
	// Concept: _f must be Callable, must accept N parameters of type T&, must return optional<vector<T>>
	detail::iterateReplacingWindow(_vector, _f, std::make_index_sequence<N>{});
}

/// @returns true iff @a _str passess the hex address checksum test.
/// @param _strict if false, hex strings with only uppercase or only lowercase letters
/// are considered valid.
bool passesAddressChecksum(std::string const& _str, bool _strict);

/// @returns the checksummed version of an address
/// @param hex strings that look like an address
std::string getChecksummedAddress(std::string const& _addr);

bool isValidHex(std::string const& _string);
bool isValidDecimal(std::string const& _string);

/// @returns a quoted string if all characters are printable ASCII chars,
/// or its hex representation otherwise.
std::string formatAsStringOrNumber(std::string const& _value);

template<typename Container, typename Compare>
bool containerEqual(Container const& _lhs, Container const& _rhs, Compare&& _compare)
{
	return std::equal(std::begin(_lhs), std::end(_lhs), std::begin(_rhs), std::end(_rhs), std::forward<Compare>(_compare));
}

inline std::string findAnyOf(std::string const& _haystack, std::vector<std::string> const& _needles)
{
	for (std::string const& needle: _needles)
		if (_haystack.find(needle) != std::string::npos)
			return needle;
	return "";
}


namespace detail
{
template<typename T>
void variadicEmplaceBack(std::vector<T>&) {}
template<typename T, typename A, typename... Args>
void variadicEmplaceBack(std::vector<T>& _vector, A&& _a, Args&&... _args)
{
	_vector.emplace_back(std::forward<A>(_a));
	variadicEmplaceBack(_vector, std::forward<Args>(_args)...);
}
}

template<typename T, typename... Args>
std::vector<T> make_vector(Args&&... _args)
{
	std::vector<T> result;
	result.reserve(sizeof...(_args));
	detail::variadicEmplaceBack(result, std::forward<Args>(_args)...);
	return result;
}

}
