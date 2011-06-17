# Desired name of the executable

EXEC = nodes


# Add/Remove any flags to the compiler here..

INCLUDEDIRS = include/ 

CFLAGS = -Wall -I $(INCLUDEDIRS) -nostdinc -nostdlib -fno-builtin -fno-exceptions \
	-Winline



# The architecture we are compiling for..

ARCH = i386
ARCHDIR = arch/$(ARCH)

# List the object files your exectuable depends on here..
# If you add more object files, remember to add a line specifying
# which source files the object file in question depends on.


OBJFILES = $(ARCHDIR)/boot/boot.o				  \
	$(ARCHDIR)/mm/init.o					  \
	mm/page_alloc.o						  \
	kernel/print.o						  \
	kernel/main.o						  \
	$(ARCHDIR)/kernel/i8259.o				  \
	$(ARCHDIR)/kernel/interrupts.o				  \
	$(ARCHDIR)/kernel/irq.o					  \
	$(ARCHDIR)/drivers/keyboard.o


$(EXEC) : $(OBJFILES)
	$(LD) -T nodes.ld -o $(EXEC) $(OBJFILES)

i8259.o : $(ARCHDIR)/kernel/i8259.S
	$(AS) -o i8259.o $(ARCHDIR)/kernel/i8259.S


$(ARCHDIR)/drivers/keyboard.o : include/sys/types.h include/nodes/keymap.h \
				include/io.h include/asm/io.h include/asm/interrupt.h


kernel/main.o : include/asm/interrupt.h include/io.h include/nodes/devices.h \
		include/multiboot.h 

kernel/print.o : include/io.h


boot.o : $(ARCHDIR)/boot/boot.S
	$(AS) -o boot.o $(ARCHDIR)/boot/boot.S


$(ARCHDIR)/kernel/interrupts.o : include/sys/types.h include/asm/interrupt.h \
				include/asm/io.h include/io.h

irq.o : $(ARCHDIR)/kernel/irq.S
	$(AS) -o irq.o irq.S

mm/page_alloc.o : include/sys/types.h include/mm/mm.h include/io.h 

$(ARCHDIR)/mm/init.o : include/sys/types.h include/mm/mm.h include/asm/mm.h include/asm/gdt.h

clean :
	rm $(OBJFILES) 
cleanall : 
	rm $(OBJFILES) $(EXEC)

