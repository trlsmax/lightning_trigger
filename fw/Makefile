TARGET=lightning.ihx
SDCC_PATH=~/bin/sdcc/bin
SDCC=$(SDCC_PATH)/sdcc
SDLD=$(SDCC_PATH)/sdld
PACKIHX=$(SDCC_PATH)/packihx

.PHONY: all clean flash

all: $(TARGET)

clean:
	rm -f $(OBJECTS) *.asm *.cdb *.lk *.lst *.map *.rel *.rst *.s19 *.s37 *.sym *.ihx *.hex

flash: $(TARGET)
	stm8flash -cstlinkv2 -pstm8s105 -w $(TARGET)

%.hex: %.ihx
	@$(PACKIHX) $< > $@

%.ihx: %.c
	$(SDCC) -DSTM8S105 -lstm8 -mstm8 --std-sdcc99 --out-fmt-ihx $(CFLAGS) $(LDFLAGS) $<
