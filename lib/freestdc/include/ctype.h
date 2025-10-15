#ifndef _FREESTDC_CTYPE_H_
#define _FREESTDC_CTYPE_H_

inline int isalnum(int c);
inline int isalpha(int c);
inline int iscntrl(int c);
inline int isdigit(int c);
inline int isgraph(int c);
inline int islower(int c);
inline int isprint(int c);
inline int ispunct(int c);
inline int isspace(int c);
inline int isupper(int c);
inline int isxdigit(int c);

inline int tolower(int c);
inline int toupper(int c);

inline int isalnum(int c) {
	return (isalpha(c) || isdigit(c));
}

inline int isalpha(int c) {
	return (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')));
}

inline int iscntrl(int c) {
	return ((c >= 0 && c <= 31) || c == 127);
}

inline int isdigit(int c) {
	return ((c >= '0') && (c <= '9'));
}

inline int isgraph(int c) {
	return !(iscntrl(c) || isspace(c));
}

inline int islower(int c) {
	return (c >= 'a' && c <= 'z');
}

inline int isprint(int c) {
	return !iscntrl(c);
}

inline int ispunct(int c) {
	return ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c == '~'));
}

inline int isspace(int c) {
	return c == ' ';
}

inline int isupper(int c) {
	return (c >= 'A' && c <= 'Z');
}

inline int isxdigit(int c) {
	return (isdigit(c) || ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')));
}

inline int tolower(int c) {
	if (isupper(c))
		return c - 32;
	return c;
}

inline int toupper(int c) {
	if (islower(c))
		return c + 32;
	return c;
}

#endif
