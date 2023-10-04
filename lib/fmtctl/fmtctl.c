#include "fmtctl.h"

///
/// @brief Compile a format control information block from a string.
///
/// @param str Input string.
/// @param dest Where to store the result.
/// @return Count of scanned characters. 0 for failed.
///
size_t fmtctl_parse(const char *str, fmtctl_info_t *dest)
{
	size_t n_scanned = 0;

	if (str[0] != '%')
		return 0;

	n_scanned++;

	// Flags
	switch (str[n_scanned])
	{
	case '-':
		dest->flags = FMTCTL_FLAGS_RIGHT_ALIGN;
		n_scanned++;
		break;
	case '+':
		dest->flags = FMTCTL_FLAGS_DISPLAY_POSITIVE;
		n_scanned++;
		break;
	case '#':
		dest->flags = FMTCTL_FLAGS_SPECIAL_OUTPUT;
		n_scanned++;
		break;
	case '0':
		dest->flags = FMTCTL_FLAGS_PADDING_WITH_0;
		n_scanned++;
		break;
	case ' ':
		n_scanned++;
	default:
		dest->flags = FMTCTL_FLAGS_NONE;
	}

	// Width
	dest->width = 0;
	if (str[n_scanned] >= '0' && str[n_scanned] <= '9')
		while (str[n_scanned] >= '0' && str[n_scanned] <= '9')
		{
			dest->width *= 10;
			dest->width += str[n_scanned] - '0';
			n_scanned++;
		}

	// Precisions
	dest->precision = 0;
	if (str[n_scanned] == '.')
	{
		n_scanned++;
		while (str[n_scanned] >= '0' && str[n_scanned] <= '9')
		{
			dest->precision *= 10;
			dest->precision += str[n_scanned] - '0';
			n_scanned++;
		}
	}

	// Length
	switch (str[n_scanned])
	{
	case 'h':
		dest->length = FMTCTL_LENGTH_HALF;
		n_scanned++;
		break;
	case 'l':
		dest->length = FMTCTL_LENGTH_LONG;
		n_scanned++;
		break;
	case 'L':
		dest->length = FMTCTL_LENGTH_LONG_FLOAT;
		n_scanned++;
		break;
	default:
		dest->length = FMTCTL_LENGTH_NONE;
	}

	// Specifier
	switch (str[n_scanned])
	{
	case 'd':
		dest->specifier = FMTCTL_SPECIFIER_DEC;
		n_scanned++;
		break;
	case 'o':
		dest->specifier = FMTCTL_SPECIFIER_OCT;
		n_scanned++;
		break;
	case 'X':
	case 'x':
		dest->specifier = FMTCTL_SPECIFIER_HEX;
		n_scanned++;
		break;
	case 'u':
		dest->specifier = FMTCTL_SPECIFIER_UNSIGNED;
		n_scanned++;
		break;
	case 'f':
		dest->specifier = FMTCTL_SPECIFIER_FLOAT;
		n_scanned++;
		break;
	case 'E':
	case 'e':
		dest->specifier = FMTCTL_SPECIFIER_FLOAT_EXPONENTIAL;
		n_scanned++;
		break;
	case 'G':
	case 'g':
		dest->specifier = FMTCTL_SPECIFIER_FLOAT_TRIMMED;
		n_scanned++;
		break;
	case 'c':
		dest->specifier = FMTCTL_SPECIFIER_CHAR;
		n_scanned++;
		break;
	case 's':
		dest->specifier = FMTCTL_SPECIFIER_STR;
		n_scanned++;
		break;
	case 'p':
		dest->specifier = FMTCTL_SPECIFIER_PTR;
		n_scanned++;
		break;
	case 'l':
		n_scanned++;
		switch (str[n_scanned])
		{
		case 'u':
			dest->specifier = FMTCTL_SPECIFIER_LUNSIGNED;
			n_scanned++;
			break;
		case 'l':
			n_scanned++;
			if (str[n_scanned] != 'u')
				return 0;
			dest->specifier = FMTCTL_SPECIFIER_LLUNSIGNED;
			break;
		default:
			return 0;
		}
		dest->specifier = FMTCTL_SPECIFIER_STR;
		break;
	default:
		return 0;
	}

	return n_scanned;
}
