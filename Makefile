PROGNAME = AmongUsTeachesTyping

OFILES = gba.o font.o main.o images/accuracy.o images/amongusbg.o images/crewmatedie.o images/crewmatess.o images/ejectedcrewmate.o images/finger.o images/jermasus.o images/losescreen.o images/speechbubble.o images/titlescreen.o images/winscreen.o images/wpm.o

.PHONY: all
all: CFLAGS += $(CRELEASE) -I../shared
all: LDFLAGS += $(LDRELEASE)
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

include ./tools/GBAVariables.mak

LDFLAGS += --specs=nosys.specs

# Adjust default compiler warnings and errors
CFLAGS += -Wstrict-prototypes -Wold-style-definition

.PHONY: debug
debug: CFLAGS += $(CDEBUG) -I../shared
debug: LDFLAGS += $(LDDEBUG)
debug: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

$(PROGNAME).gba: $(PROGNAME).elf
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES) libc_sbrk.o
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: mgba
mgba: CFLAGS += $(CRELEASE) -I../shared
mgba: LDFLAGS += $(LDRELEASE)
mgba: $(PROGNAME).gba
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@mgba-qt $(PROGNAME).gba

.PHONY: gdb
gdb: debug
	@echo "[EXECUTE] Running Emulator MGBA with GDB"
	@echo "          Please see emulator.log if this fails"
	@mgba-qt $(PROGNAME).gba --gdb &
	# To give mgba some time to initialize and start up the GDB server.
	@sleep 0.5
	@gdb-multiarch -ex 'file $(PROGNAME).elf' -ex 'target remote 127.0.0.1:2345'

.PHONY: submit
submit: clean
	@rm -f submission.tar.gz
	@tar czvf submission.tar.gz *

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	rm -f *.o *.elf *.gba *.log */*.o *.sav
