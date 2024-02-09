@echo off
rem Compile main.c using SDCC
sdcc --model-small main.c

rem Flash main.ihx to STC89 microcontroller using STCGAL
stcgal -p COM8 -P stc89 -o cpu_6t_enabled=true main.ihx

rem Pause to keep the command prompt window open
pause
