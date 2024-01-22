set HEXFILE=%1

set "HEXFILE=%HEXFILE:\=/%"

openocd -f ./tools/wch-interface.cfg -f ./tools/wch-target.cfg -c init -c halt -c "flash erase_sector wch_riscv 0 last " -c "program %HEXFILE%" -c "verify_image %HEXFILE%" -c wlink_reset_resume -c exit

