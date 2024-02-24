#include "minicrt.h"

int mini_crt_io_init() { return 1; }

#ifdef WIN32
#include <Windows.h>

FILE *fopen(const char *filename, const char *mode) {
  HANDLE hFile = 0;
  int access = 0;
  int creation = 0;

  if (strcmp(mode, "w") == 0) {
    access |= GENERIC_WRITE;
    creation |= CREATE_ALWAYS;
  }

  if (strcmp(mode, "w+") == 0) {
    access |= GENERIC_WRITE | GENERIC_READ;
    creation |= CRATE_ALWAYS;
  }

  if (strcmp(mode, "r") == 0) {
    access |= GENERIC_READ;
    creation += OPEN_EXISTING;
  }

  if (strcmp(mode, "r+") == 0) {
    access |= GENERIC_WRITE | GENERIC_READ;
    creation |= TRUNCATE_EXISTING;
  }

  hFile = CreateFileA(filename, access, 0, 0, createion, 0, 0);
  if (hFile == INVALID_HANDLE_VALUE) {
    return 0;
  }

  return (FILE *)hFile;
}

int fread(void *buffer, int size, int count, FILE *stream) {
  int read = 0;
  if (!ReadFile((HANDLE)stream, buffer, size * count, &read, 0)) {
    return 0;
  }

  return read;
}

int fwrite(const void *buffer, int size, int count, FILE *stream) {
  int written = 0;
  if (!WriteFile((HANDLE)stream, buffer, size * count, &written, 0)) {
    return 0;
  }
  return written;
}

int fclose(FILE *fp) { return CloseHandle((HANDLE)fp); }

int fseek(FILE *fp, int offset, int set) {
  return SetFilePointer((HANDLE)fp, offset, 0, set);
}
#else  // #ifdef WIN32

static int open(const char *pathname, int flags, int mode) {
  int fd = 0;
  asm("movl $5, %%eax \n\t"
      "movl %1, %%ebx \n\t"
      "movl %2, %%ecx \n\t"
      "movl %3, %%edx \n\t"
      "int $0x80 \n\t"
      "movl %%eax, %0 \n\t"
      : "=m"(fd)
      : "m"(pathname), "m"(flags), "m"(mode));
}

static int read(int fd, void *buffer, unsigned size) {
  int ret = 0;
  asm("movl $3, %%eax \n\t"
      "movl %1, %%ebx \n\t"
      "movl %2, %%ecx \n\t"
      "movl %3, %%edx \n\t"
      "int $0x80 \n\t"
      "movl %%eax, %0 \n\t"
      : "=m"(ret)
      : "m"(fd), "m"(buffer), "m"(size));
  return ret;
}

static int write(int fd, const void *buffer, unsigned size) {
  int ret = 0;
  asm("movl $4, %%eax \n\t"
      "movl %1, %%ebx \n\t"
      "movl %2, %%ecx \n\t"
      "movl %3, %%edx \n\t"
      "int $0x80 \n\t"
      "movl %%eax, %0 \n\t"
      : "=m"(ret)
      : "m"(fd), "m"(buffer), "m"(size));
  return ret;
}

static int close(int fd) {
  int ret = 0;
  asm("movl $6, %%eax \n\t"
      "movl %1, %%ebx \n\t"
      "int $0x80 \n\t"
      "movl %%eax, %0 \n\t"
      : "=m"(ret)
      : "m"(fd));
  return ret;
}

static int seek(int fd, int offset, int mode) {
  int ret = 0;
  asm("movl $19, %%eax \n\t"
      "movl %1, %%ebx \n\t"
      "movl %2, %%ecx \n\t"
      "movl %3, %%edx \n\t"
      "int $0x80 \n\t"
      "movl %%eax, %0 \n\t"
      : "=m"(ret)
      : "m"(fd), "m"(offset), "m"(mode));
  return ret;
}

FILE *fopen(const char *filename, const char *mode) {
  int fd = -1;
  int flags = 0;
  int access = 00700;  // 创建文件权限

// 注意: 以0开始的数字是八进制的
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_APPEND = 02000

  if (strcmp(mode, "w") == 0) {
    flags |= O_WRONLY | O_CREAT | O_TRUNC;
  }

  if (strcmp(mode, "w+") == 0) {
    flags |= O_RDWR | O_CREAT | O_TRUNC;
  }

  if (strcmp(mode, "r") == 0) {
    flags |= O_RDONLY;
  }

  if (strcmp(mode, "r+") == 0) {
    flags |= O_RDWR | O_CREAT;
  }

  fd = open(filename, flags, access);
  return (FILE *)fd;
}

int fread(void *buffer, int size, int count, FILE *stream) {
  return read((int)stream, buffer, size * count);
}

int fwrite(const void *buffer, int size, int count, FILE *stream) {
  return write((int)stream, buffer, size * count);
}

int fclose(FILE *fp) { return close((int)fp); }

int fseek(FILE *fp, int offset, int set) { return seek((int)fp, offset, set); }
#endif

int fputc(int c, FILE *stream) {
  if (fwrite(&c, 1, 1, stream) != 1) {
    return EOF;
  } else {
    return c;
  }
}

int fputs(const char *str, FILE *stream) {
  int len = strlen(str);
  if (fwrite(str, 1, len, stream) != len) {
    return EOF;
  } else {
    return len;
  }
}

#ifndef WIN32
#define va_list char *
#define va_start(ap, arg) (ap = ((va_list) & arg) + sizeof(arg))
#define va_arg(ap, t) (*(t *)((ap += sizeof(t)) - sizeof(t)))
#define va_end(ap) (ap = (va_list)0)
#else
#include <Windows.h>
#endif

int vfprintf(FILE *stream, const char *format, va_list arglist) {
  int translating = 0;
  int ret = 0;
  const char *p = 0;
  for (p = format; *p != '\0'; ++p) {
    switch (*p) {
      case '%': {
        if (!translating) {
          translating = 1;
        } else {
          if (fputc('%', stream) < 0) {
            return EOF;
          }
          ++ret;
          translating = 0;
        }
        break;
      }
      case 'd': {
        if (translating) {
          char buf[16];
          translating = 0;
          itoa(va_arg(arglist, int), buf, 10);
          if (fputs(buf, stream) < 0) {
            return EOF;
          }
          ret += strlen(buf);
        } else if (fputc('d', stream) < 0) {
          return EOF;
        } else {
          ++ret;
        }
        break;
      }
      case 's': {
        if (translating) {
          const char *str = va_arg(arglist, const char *);
          translating = 0;
          if (fputs(str, stream) < 0) {
            return EOF;
          }
          ret += strlen(str);
        } else if (fputc('s', stream) < 0) {
          return EOF;
        } else {
          ++ret;
        }
        break;
      }
      default: {
        if (translating) {
          translating = 0;
        }

        if (fputc(*p, stream) < 0) {
          return EOF;
        } else {
          ret++;
        }
        break;
      }
    }
  }

  return ret;
}

int printf(const char *format, ...) {
  va_list arglist;
  va_start(arglist, format);
  return vfprintf(stdout, format, arglist);
}

int fprintf(FILE *stream, const char *format, ...) {
  va_list arglist;
  va_start(arglist, format);
  return vfprintf(stream, format, arglist);
}