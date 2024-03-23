@echo off

mkdir ..\..\build
pushd ..\..\build
cl -Zi ..\handmade\code\handmade.cpp User32.lib Gdi32.lib Ole32.lib
popd
