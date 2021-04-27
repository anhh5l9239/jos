// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
//#include "fs.h"
//#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
} cons;

static void printint(int xx, int base, int sign) {
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	uint x;

	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (sign)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char *fmt, ...) {
	int i, c/*, locking*/;
	uint *argp;
	char *s;

//  locking = cons.locking;
//  if(locking)
	acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	argp = (uint*) (void*) (&fmt + 1);
	for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
		if (c != '%') {
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if (c == 0)
			break;
		switch (c) {
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if ((s = (char*) *argp++) == 0)
				s = "(null)";
			for (; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}

//  if(locking)
	release(&cons.lock);
}

void panic(char *s) {
	int i;
	uint pcs[10];

	cli();
	cons.locking = 0;
	cprintf("cpu%d: panic: ", cpu->id);
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for (i = 0; i < 10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for (;;)
		;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4

void consputc(int c) {
	if (panicked) {
		cli();
		for (;;)
			;
	}
	uartputc(c);
}

void consoleintr(int (*getc)(void)) {
	int c;

	while ((c = getc()) >= 0) {
		switch (c) {
		case 'p':  // Process listing.
			procdump();
			break;
		default:
			break;
		}
	}
}

