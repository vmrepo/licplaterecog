rmdir /s /q %~dp0.vs
rmdir /s /q %~dp0Debug
rmdir /s /q %~dp0ipch
rmdir /s /q %~dp0x64
rmdir /s /q %~dp0licplate\x64
del %~dp0licplate\*.user*
del %~dp0licplate\*.cd
del %~dp0licplate\*.aps
del %~dp0*.db
del %~dp0*.opendb
