/* Modified version of the Minix keyboard driver. The license for this
 * code can be found at : SRCDIR/LICENSE.MINIX
 */

/* Keyboard driver for PC's and AT's.
 *
 * Changed by Marcus Hampel	(04/02/1994)
 *  - Loadable keymaps
 */

#include <sys/types.h>
#include <nodes/keymap.h>
#include "keymaps/us-std.src"
#include <io.h>
#include <asm/io.h>
#include <asm/interrupt.h>

/* Standard and AT keyboard.  (PS/2 MCA implies AT throughout.) */
#define KEYBD		0x60	/* I/O port for keyboard data */

/* AT keyboard. */
#define KB_COMMAND	0x64	/* I/O port for commands on AT */
#define KB_GATE_A20	0x02	/* bit in output port to enable A20 line */
#define KB_PULSE_OUTPUT	0xF0	/* base for commands to pulse output port */
#define KB_RESET	0x01	/* bit in output port to reset CPU */
#define KB_STATUS	0x64	/* I/O port for status on AT */
#define KB_ACK		0xFA	/* keyboard ack response */
#define KB_BUSY		0x02	/* status bit set when KEYBD port ready */
#define LED_CODE	0xED	/* command to keyboard to set LEDs */
#define MAX_KB_ACK_RETRIES 0x1000	/* max #times to wait for kb ack */
#define MAX_KB_BUSY_RETRIES 0x1000	/* max #times to loop while kb busy */
#define KBIT		0x80	/* bit used to ack characters to keyboard */
#define PORT_B          0x61
/* Miscellaneous. */
#define ESC_SCAN	   1	/* Reboot key when panicking */
#define SLASH_SCAN	  53	/* to recognize numeric slash */
#define HOME_SCAN	  71	/* first key on the numeric keypad */
#define DEL_SCAN	  83	/* DEL for use in CTRL-ALT-DEL reboot */

#define KB_IRQ             1     /* Our Keyboard IRQ number is 1 */

#define KB_IN_BYTES	  32	/* size of keyboard input buffer */

static int alt1;		/* left alt key state */
static int alt2;		/* right alt key state */
static int capslock;		/* caps lock key state */
static int esc;		/* escape scan code detected? */
static int control;		/* control key state */
static int caps_off;		/* 1 = normal position, 0 = depressed */
static int numlock;		/* number lock key state */
static int num_off;		/* 1 = normal position, 0 = depressed */
static int slock;		/* scroll lock key state */
static int slock_off;		/* 1 = normal position, 0 = depressed */
static int shift;		/* shift key state */

static char numpad_map[] =
{'H', 'Y', 'A', 'B', 'D', 'C', 'V', 'U', 'G', 'S', 'T', '@'};

/* Keyboard structure, 1 per console. */
struct kb_s {
	char *ihead;			/* next free spot in input buffer */
	char *itail;			/* scan code to return to TTY */
	int icount;			/* # codes in buffer */
	char ibuf[KB_IN_BYTES];	/* input buffer */
};

static struct kb_s keyb;

static int kb_ack (void);
static int kb_wait (void);
static int scan_keyboard (void);
static u32_t make_break (int scode);
static void set_leds (void);
void kbd_hw_int (void);
static void kb_read (void);
static unsigned map_key (int scode);


/*===========================================================================*
 *				map_key0				     *
 *===========================================================================*/
/* Map a scan code to an ASCII code ignoring modifiers. */
#define map_key0(scode)				\
	((unsigned) keymap[(scode) * MAP_COLS])


/*===========================================================================*
 *				map_key					     *
 *===========================================================================*/
static unsigned map_key(int scode)
{
/* Map a scan code to an ASCII code. */

	int caps, column;
	u16_t *keyrow;

	if (scode == SLASH_SCAN && esc) return '/';	/* don't map numeric slash */

	keyrow = &keymap[scode * MAP_COLS];

	caps = shift;
	if (numlock && HOME_SCAN <= scode && scode <= DEL_SCAN) caps = !caps;
	if (capslock && (keyrow[0] & HASCAPS)) caps = !caps;

	if (alt1 || alt2) {
		column = 2;
		if (control || alt2) column = 3;	/* Ctrl + Alt1 == Alt2 */
		if (caps) column = 4;
	} else {
		column = 0;
		if (caps) column = 1;
		if (control) column = 5;
	}
	return keyrow[column] & ~HASCAPS;
}


/*===========================================================================*
 *				kbd_hw_int				     *
 *===========================================================================*/
void kbd_hw_int(void)
{
/* A keyboard interrupt has occurred.  Process it. */

	int code;
	unsigned km;
	
	/* Fetch the character from the keyboard hardware and acknowledge it. */
	code = scan_keyboard();

	/* The IBM keyboard interrupts twice per key, once when depressed, once when
	 * released.  Filter out the latter, ignoring all but the shift-type keys.
	 * The shift-type keys 29, 42, 54, 56, 58, and 69 must be processed normally.
	 */

	if (code & 0200) {
		/* A key has been released (high bit is set). */
		km = map_key0(code & 0177);
		if (km != CTRL && km != SHIFT && km != ALT && km != CALOCK
		    && km != NLOCK && km != SLOCK && km != EXTKEY)
			return ;
	}

	/* Store the character in memory so the task can get at it later. */
	if (keyb.icount < KB_IN_BYTES) {
		*keyb.ihead++ = code;
		if (keyb.ihead == keyb.ibuf + KB_IN_BYTES) keyb.ihead = keyb.ibuf;
		keyb.icount++;

	} 

	kb_read();
	/* Else it doesn't fit - discard it. */
/*  return 1;	 Reenable keyboard interrupt */
}


/* NOTE (6/1/2004) "Apurva Mehta" <apurva@gmx.net> : The following
 * function has been modified from the minix original because I still
 * do not have a multi tasking infrastructure. Whenever you encounter
 * a `putchar', it means that the character is in ASCII and is ready
 * to be sent to whoever has asked for it.
 */

/*==========================================================================*
 *				kb_read					    *
 *==========================================================================*/
static void kb_read()
{
/* Process characters from the circular keyboard buffer. */

	int scode;
	u32_t ch;

  
	while (keyb.icount > 0) {
		scode = *keyb.itail++;			/* take one key scan code */
		if (keyb.itail == keyb.ibuf + KB_IN_BYTES) keyb.itail = keyb.ibuf;
		keyb.icount--;
	

		/* Perform make/break processing. */
		ch = make_break(scode);

		if (ch <= 0xFF) {
			/* A normal character. */
			putchar (ch);

		}
		/* Not checking for ANSI escape sequences for now */
	}
}

/*===========================================================================*
 *				make_break				     *
 *===========================================================================*/
static u32_t make_break(int scode)
{
/* This routine can handle keyboards that interrupt only on key depression,
 * as well as keyboards that interrupt on key depression and key release.
 * For efficiency, the interrupt routine filters out most key releases.
 */
	int ch, make;
	static int CAD_count = 0;

	/* Check for CTRL-ALT-DEL, and if found, halt the computer. This would
	 * be better done in keyboard() in case TTY is hung, except control and
	 * alt are set in the high level code.
	 */
	if (control && (alt1 || alt2) && scode == DEL_SCAN)
	{

		if (++CAD_count == 3){
			kb_wait();
			outb( KB_COMMAND, 
			      KB_PULSE_OUTPUT | (0x0F & ~(KB_GATE_A20 | KB_RESET)));
		}

		return -1;
	}

	/* High-order bit set on key release. */
	make = (scode & 0200 ? 0 : 1);	/* 0 = release, 1 = press */

	ch = map_key(scode & 0177);		/* map to ASCII */

	switch (ch) {
  	case CTRL:
		control = make;
		ch = -1;
		break;
  	case SHIFT:
		shift = make;
		ch = -1;
		break;
  	case ALT:
		if (make) {
			if (esc) alt2 = 1; else alt1 = 1;
		} else {
			alt1 = alt2 = 0;
		}
		ch = -1;
		break;
  	case CALOCK:
		if (make && caps_off) {
			capslock = 1 - capslock;
			set_leds();
		}
		caps_off = 1 - make;
		ch = -1;
		break;
  	case NLOCK:
		if (make && num_off) {
			numlock = 1 - numlock;
			set_leds();
		}
		num_off = 1 - make;
		ch = -1;
		break;
  	case SLOCK:
		if (make & slock_off) {
			slock = 1 - slock;
			set_leds();
		}
		slock_off = 1 - make;
		ch = -1;
		break;
  	case EXTKEY:
		esc = 1;
		return(-1);
  	default:
		if (!make) ch = -1;
	}
	esc = 0;
	return(ch);
}

/*===========================================================================*
 *				set_leds				     *
 *===========================================================================*/
static void set_leds()
{
/* Set the LEDs on the caps lock and num lock keys */

	unsigned leds;

	/* encode LED bits */
	leds = (slock << 0) | (numlock << 1) | (capslock << 2);

	kb_wait();			/* wait for buffer empty  */
	outb (LED_CODE, KEYBD);	/* prepare keyboard to accept LED values */
	kb_ack();			/* wait for ack response  */

	kb_wait();			/* wait for buffer empty  */
	outb(leds, KEYBD);	/* give keyboard LED values */
	kb_ack();			/* wait for ack response  */
}


/*==========================================================================*
 *				kb_wait					    *
 *==========================================================================*/
static int kb_wait()
{
/* Wait until the controller is ready; return zero if this times out. */

	int retries;

	retries = MAX_KB_BUSY_RETRIES + 1;
	while (--retries != 0 && inb(KB_STATUS) & KB_BUSY)
		;			/* wait until not busy */
	return(retries);		/* nonzero if ready */
}


/*==========================================================================*
 *				kb_ack					    *
 *==========================================================================*/
static int kb_ack()
{
/* Wait until kbd acknowledges last command; return zero if this times out. */

	int retries;

	retries = MAX_KB_ACK_RETRIES + 1;
	while (--retries != 0 && inb(KEYBD) != KB_ACK)
		;			/* wait for ack */
	return(retries);		/* nonzero if ack received */
}

/*===========================================================================*
 *				kb_init					     *
 *===========================================================================*/
void kb_init()
{
/* Initialize the keyboard driver. */

	/* Set up input queue. */
	keyb.ihead = keyb.itail = keyb.ibuf;

	/* Set initial values. */
	caps_off = 1;
	num_off = 1;
	slock_off = 1;
	esc = 0;

	set_leds();			/* turn off numlock led */

	scan_keyboard();		/* stop lockup from leftover keystroke */

 	set_irq_handler( KB_IRQ, kbd_hw_int);	/*  set the interrupt handler */
 	enable_irq( KB_IRQ );	/* safe now everything initialised! */
}




/*==========================================================================*
 *				scan_keyboard				    *
 *==========================================================================*/
static int scan_keyboard()
{
/* Fetch the character from the keyboard hardware and acknowledge it. */

	int code;
	int val;

	code = inb(KEYBD);	/* get the scan code for the key struck */
	val = inb(PORT_B);	/* strobe the keyboard to ack the char */
	outb( val | KBIT, PORT_B);	/* strobe the bit high */
	outb(val, PORT_B);	/* now strobe it low */
	return code;
}

 
