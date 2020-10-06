@echo off
cd /d %~dp0
regsvr32 Bin\x64Release\RadAudioMeterDB.dll
