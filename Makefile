##
# Generic makefile
#

#############################################################
CC = gcc # Compilador

CPP_FLAGS =         
C_FLAGS   = 
LD_FLAGS  = 

executable = mtool
sources = mtool.c
#############################################################

objects = $(sources:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(CPP_FLAGS) -c $<

$(executable) : $(objects)
	$(CC) $(LD_FLAGS) $(LDFLAGS) $(objects) -o $(executable)

.Makefile.dep: *.c
	@$(CC) $(CFLAGS) $(CPP_FLAGS) -MM *.c > $@

-include .Makefile.dep

##
# clean
#
.PHONY clean:
	@rm -f *.o \#* *~  .Makefile.dep
	@rm -f $(executable)

