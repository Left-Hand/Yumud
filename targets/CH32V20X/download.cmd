@REM set HEXFILE=%1

@REM set "HEXFILE=%HEXFILE:\=/%"

@REM openocd -f ./tools/wch-interface.cfg -f ./tools/wch-target.cfg -c page_erase -c init -c halt -c "flash erase_sector wch_riscv 0 last " -c "program %HEXFILE%" -c "verify_image %HEXFILE%" -c wlink_reset_resume -c exit

@REM openocd -f ./tools/wch-interface.cfg  -c page_erase  -c init -c halt  -c "flash write_image %HEXFILE%"   -c exit

set HEXFILE=%1

set "HEXFILE=%HEXFILE:\=/%"

@REM openocd -f ./tools/wch-interface.cfg -f ./tools/wch-target.cfg -c page_erase -c init -c halt -c "flash erase_sector wch_riscv 0 last " -c "program %HEXFILE%" -c wlink_reset_resume -c exit
openocd -f ./tools/wch-interface.cfg -f ./tools/wch-target.cfg -c init -c halt -c "flash erase_sector wch_riscv 0 last " -c "program %HEXFILE%" -c wlink_reset_resume -c exit