@echo needs:
@echo  fw.hex
@echo  xboot.hex
@echo generates:
@echo  analyzer_PIC18F26K42.combined.hex
@echo  analyzer_PIC18F26K42.firmware.hex
@echo  analyzer_PIC18F26K42.xbootbootloader.hex
@echo  analyzer_PIC18F26K42.firmware.xboot
@echo Conflicting config bits will result in      xboot.hex      configbits
@echo All resulting files will contain same config bits

srec_cat fw.hex -I xboot.hex -I -o analyzer_PIC18F26K42.combined.hex -I
srec_cat analyzer_PIC18F26K42.combined.hex -I -crop -minimum-addr fw.hex -I -o analyzer_PIC18F26K42.firmware.hex -I
srec_cat analyzer_PIC18F26K42.combined.hex -I -exclude ( -minimum-addr fw.hex -I 0xFFFF ) -o analyzer_PIC18F26K42.xbootbootloader.hex -I
srec_cat fw.hex -I -offset - -minimum-addr fw.hex -I -o analyzer_PIC18F26K42.firmware.xboot -bin