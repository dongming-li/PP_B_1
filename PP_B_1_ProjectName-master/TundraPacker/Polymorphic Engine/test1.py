#!/usr/bin/python

from capstone import *
from keystone import *

CODE = "\x55\x48\x8b\x05\xb8\x13\x00\x00"

try:
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    for i in md.disasm(CODE, 0x1000):
	    print i.mnemonic

except CsError as e:
    print("ERROR: %s" %e)


CODE = b"INC ecx; DEC edx"

try:
   # Initialize engine in X86-32bit mode
   ks = Ks(KS_ARCH_X86, KS_MODE_32)
   encoding, count = ks.asm(CODE)

   print("%s = %s (number of statements: %u)" %(CODE, encoding, count))
except KsError as e:
   print("ERROR: %s" %e)

