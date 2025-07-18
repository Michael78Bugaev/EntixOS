#include <vga.h>
#include <port_based.h>
#include <stdint.h>
#include <stdarg.h>

static void	vga_memcpy(uint8_t *src, uint8_t *dest, int bytes);
static void	vga_memcpy(uint8_t *src, uint8_t *dest, int bytes)
{
	int i;

	i = 0;
	while (i < bytes)
	{
		dest[i] = src[i];
		i++;
	}
}

void	kprint(uint8_t *str)
{
	while (*str)
	{
		putchar(*str, WHITE_ON_BLACK);
		str++;
	}
}

void	putchar(uint8_t character, uint8_t attribute_byte)
{
	uint16_t offset;
	offset = get_cursor();
	if (character == '\n')
	{
		if ((offset / 2 / MAX_COLS) == (MAX_ROWS - 1)) 
			scroll_line();
		else
			set_cursor((offset - offset % (MAX_COLS*2)) + MAX_COLS*2);
	}
    else if (character == '\b')
    {
        set_cursor(get_cursor() - 1);
        putchar(' ', attribute_byte);
        set_cursor(get_cursor() - 2);
    }
	else 
	{
		if (offset == (MAX_COLS * MAX_ROWS * 2)) scroll_line();
		write(character, attribute_byte, offset);
		set_cursor(offset+2);
	}
}

void	scroll_line()
{
	uint8_t i = 1;
	uint16_t last_line;
	while (i < MAX_ROWS)
	{
		vga_memcpy((uint8_t *)(VIDEO_ADDRESS + (MAX_COLS * i * 2)),
			   (uint8_t *)(VIDEO_ADDRESS + (MAX_COLS * (i - 1) * 2)),
			   (MAX_COLS*2)
		);
		i++;
	}

	last_line = (MAX_COLS*MAX_ROWS*2) - MAX_COLS*2;
	i = 0;
	while (i < MAX_COLS)
	{
		write('\0', WHITE_ON_BLACK, (last_line + i * 2));
		i++;
	}
	set_cursor(last_line - 160);
}

void	clear_screen()
{
	uint16_t	offset = 0;
	while (offset < (MAX_ROWS * MAX_COLS * 2))
	{
		write('\0', WHITE_ON_BLACK, offset);
		offset += 2;
	}
	set_cursor(0);
}

void	write(uint8_t character, uint8_t attribute_byte, uint16_t offset)
{
	uint8_t *vga = (uint8_t *) VIDEO_ADDRESS;
	vga[offset] = character;
	vga[offset + 1] = attribute_byte;
}

uint16_t		get_cursor()
{
	outb(REG_SCREEN_CTRL, 14);
	uint8_t high_byte = inb(REG_SCREEN_DATA);
	outb(REG_SCREEN_CTRL, 15);
	uint8_t low_byte = inb(REG_SCREEN_DATA);
	return (((high_byte << 8) + low_byte) * 2);
}

void	set_cursor(uint16_t pos)
{
	pos /= 2;

	outb(REG_SCREEN_CTRL, 14);
	outb(REG_SCREEN_DATA, (uint8_t)(pos >> 8));
	outb(REG_SCREEN_CTRL, 15);
	outb(REG_SCREEN_DATA, (uint8_t)(pos & 0xff));
}

uint8_t get_cursor_x() {
    uint16_t pos = get_cursor();
    return pos % 80;
}

uint8_t get_cursor_y() {
    uint16_t pos = get_cursor();
    return pos / 80;
}

void set_cursor_xy(uint8_t x, uint8_t y) {
    uint16_t pos;
    
    if (x >= MAX_ROWS) x = MAX_ROWS - 1;
    if (y >= MAX_COLS) y = MAX_COLS - 1;
    
    pos = y * MAX_ROWS + x;
    set_cursor(pos);
}

void disable_cursor()
{
	outb(REG_SCREEN_CTRL, 0x0A);
	outb(REG_SCREEN_DATA, 0x20);
}
void kprint_hex(uint32_t value) {

    char hex_str[9];
    hex_str[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        uint8_t digit = value & 0xF;
        if (digit < 10) {
            hex_str[i] = '0' + digit;
        } else {
            hex_str[i] = 'A' + (digit - 10);
        }
        value >>= 4;
    }

    for (int i = 0; i < 8; i++) {
        putchar(hex_str[i],  0x07);
    }
}
void kprintc(uint8_t *str, uint8_t attr)
{
    while (*str)
	{
		if (*str == '\b')
		{
			putchar(' ', attr);
			set_cursor(get_cursor() - 2);
		}
		else {
			putchar(*str, attr);
            set_cursor(get_cursor()+1);
		}
		str++;
	}
}
void int_to_str(int num, char *str);
void int_to_str(int num, char *str) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

void kprinti(int number) {
    char buffer[12];
    int_to_str(number, buffer);
	kprint(buffer);
}

void kprintci(int number, uint8_t attr) {
    char buffer[12];
	int_to_str(number, buffer);
    kprintc(buffer, attr);
}

void kprinti_vidmem(int number, int offset) {
    char buffer[12];
    int_to_str(number, buffer);
    volatile char *video = (volatile char *)0xB8000;
    for (int i = 0; buffer[i] != '\0'; i++) {
        video[offset + i * 2] = buffer[i];
        video[offset + i * 2 + 1] = 0x07;
    }
}

void kprintci_vidmem(int number, uint8_t attr, int offset) {
    
}
void kprint_hex_w(uint32_t value) {
    char hex_str[5];
    hex_str[4] = '\0';

    for (int i = 3; i >= 0; i--) {
        uint8_t digit = value & 0xF;
        if (digit < 10) {
            hex_str[i] = '0' + digit;
        } else {
            hex_str[i] = 'A' + (digit - 10);
        }
        value >>= 4;
    }

    for (int i = 0; i < 4; i++) {
        putchar(hex_str[i], 0x07);
        set_cursor(get_cursor()+2);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[32];
    char ch;
    while (*fmt) {
        if (*fmt != '%') {
            putchar(*fmt++, WHITE_ON_BLACK);
            continue;
        }
        fmt++;
        int width = 0, zero_pad = 0;
        if (*fmt == '0') { zero_pad = 1; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') { width = width * 10 + (*fmt++ - '0'); }
        switch (*fmt) {
            case 'd': {
                int val = va_arg(args, int);
                int is_neg = val < 0;
                unsigned int uval = is_neg ? -val : val;
                int i = 0;
                do { buf[i++] = '0' + (uval % 10); uval /= 10; } while (uval);
                if (is_neg) buf[i++] = '-';
                while (i < width) buf[i++] = zero_pad ? '0' : ' ';
                while (i--) putchar(buf[i], WHITE_ON_BLACK);
                break;
            }
            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                int i = 0;
                do { buf[i++] = '0' + (val % 10); val /= 10; } while (val);
                while (i < width) buf[i++] = zero_pad ? '0' : ' ';
                while (i--) putchar(buf[i], WHITE_ON_BLACK);
                break;
            }
            case 'x': case 'X': {
                unsigned int val = va_arg(args, unsigned int);
                int i = 0;
                do {
                    int digit = val & 0xF;
                    buf[i++] = (digit < 10) ? ('0' + digit) : ((*fmt == 'x' ? 'a' : 'A') + digit - 10);
                    val >>= 4;
                } while (val);
                while (i < width) buf[i++] = zero_pad ? '0' : ' ';
                while (i--) putchar(buf[i], WHITE_ON_BLACK);
                break;
            }
            case 'p': {
                unsigned long val = (unsigned long)va_arg(args, void*);
                putchar('0', WHITE_ON_BLACK); putchar('x', WHITE_ON_BLACK);
                int i = 0;
                do {
                    int digit = val & 0xF;
                    buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                    val >>= 4;
                } while (val);
                while (i--) putchar(buf[i], WHITE_ON_BLACK);
                break;
            }
            case 'c': {
                ch = (char)va_arg(args, int);
                putchar(ch, WHITE_ON_BLACK);
                break;
            }
            case 's': {
                char *s = va_arg(args, char*);
                int len = 0;
                while (s[len]) len++;
                int pad = width - len;
                while (pad-- > 0) putchar(zero_pad ? '0' : ' ', WHITE_ON_BLACK);
                while (*s) putchar(*s++, WHITE_ON_BLACK);
                break;
            }
            case '%': {
                putchar('%', WHITE_ON_BLACK);
                break;
            }
            default:
                putchar('%', WHITE_ON_BLACK);
                putchar(*fmt, WHITE_ON_BLACK);
                break;
        }
        fmt++;
    }
    va_end(args);
}