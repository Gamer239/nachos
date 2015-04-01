# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as

# UCSD OSTL (Linux)
# GCCDIR = /home/ostl/cs120f/public/mips-x86.linux-xgcc/
# UCSD uAPE (Sparc/Solaris)
# GCCDIR = /home/solaris/ieng9/cs120f/public/mips-sparc.solaris-xgcc/
#GCCDIR = $(CSE120_GCC)
GCCDIR = /classes/cs4411/code/mips-x86.linux-xgcc/
LDFLAGS = -T script -N
ASFLAGS = -mips2
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)

PATH = $(GCCDIR):/lib:/usr/bin:/bin

CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = gcc -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt shell matmult sort syscall create read_write_test retadd simple1 simple2 simple3 simple4 simple5

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.c > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

simple1.o: simple1.c
	$(CC) $(CFLAGS) -c simple1.c
simple1: simple1.o start.o
	$(LD) $(LDFLAGS) start.o simple1.o -o simple1.coff
	../bin/coff2noff simple1.coff simple1


simple2.o: simple2.c
	$(CC) $(CFLAGS) -c simple2.c
simple2: simple2.o start.o
	$(LD) $(LDFLAGS) start.o simple2.o -o simple2.coff
	../bin/coff2noff simple2.coff simple2


simple3.o: simple3.c
	$(CC) $(CFLAGS) -c simple3.c
simple3: simple3.o start.o
	$(LD) $(LDFLAGS) start.o simple3.o -o simple3.coff
	../bin/coff2noff simple3.coff simple3


simple4.o: simple4.c
	$(CC) $(CFLAGS) -c simple4.c
simple4: simple4.o start.o
	$(LD) $(LDFLAGS) start.o simple4.o -o simple4.coff
	../bin/coff2noff simple4.coff simple4


simple5.o: simple5.c
	$(CC) $(CFLAGS) -c simple5.c
simple5: simple5.o start.o
	$(LD) $(LDFLAGS) start.o simple5.o -o simple5.coff
	../bin/coff2noff simple5.coff simple5

retadd.o: retadd.c
	$(CC) $(CFLAGS) -c retadd.c
retadd: retadd.o start.o
	$(LD) $(LDFLAGS) start.o retadd.o -o retadd.coff
	../bin/coff2noff retadd.coff retadd

read_write_test.o: read_write_test.c
	$(CC) $(CFLAGS) -c read_write_test.c
read_write_test: read_write_test.o start.o
	$(LD) $(LDFLAGS) start.o read_write_test.o -o read_write_test.coff
	../bin/coff2noff read_write_test.coff read_write_test

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

syscall.o: syscall.c
	$(CC) $(CFLAGS) -c syscall.c
syscall: syscall.o start.o
	$(LD) $(LDFLAGS) start.o syscall.o -o syscall.coff
	../bin/coff2noff syscall.coff syscall

create.o: create.c
	$(CC) $(CFLAGS) -c create.c
create: create.o start.o
	$(LD) $(LDFLAGS) start.o create.o -o create.coff
	../bin/coff2noff create.coff create
