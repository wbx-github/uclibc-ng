#include <stddef.h>

//FIXME out gets silently truncated if outsize is too small

size_t escape(char* in, char* out, size_t outsize) {
	size_t l = 0;
	while(*in && l + 3 < outsize) {
		switch(*in) {
			case '\n':
			case '\r':
			case '\t':
			case '\v':
			case '\?':
			case '\f':
				*out++ = '\\';
				l++;
				*out = in[1];
				break;
			case '\\':
			case '"':
				*out++ = '\\';
				l++;
				*out = in[0];
				break;
			default:
				*out = *in;
		}
		in++;
		out++;
		l++;
	}
	*out = 0;
	return l;
}
#include <assert.h>
#include <stdlib.h>
size_t unescape(char* in, char *out, size_t outsize) {
	size_t l = 0;
	while(*in && l + 2 < outsize) {
		switch (*in) {
			case '\\':
				++in;
				assert(*in);
				switch(*in) {
					case 'n':
						*out='\n';
						break;
					case 'r':
						*out='\r';
						break;
					case 't':
						*out='\t';
						break;
					case 'v':
						*out='\v';
						break;
					case '\?':
						*out = '\?';
						break;
					case 'f':
						*out = '\f';
						break;
					case 'b':
						*out = '\b';
						break;
					case '\\':
					case '"':
					case '\'':
						*out=*in;
						break;
					// FIXME add handling of hex and octal
					default:
						abort();
				}
				break;
			default:
				*out=*in;
		}
		in++;
		out++;
		l++;
	}
	*out = 0;
	return l;
}

