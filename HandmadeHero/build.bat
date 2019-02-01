@echo off

IF NOT EXIST build mkdir build
pushd build
set CommonCompilerFlags=/MT /nologo /Gm- /GR- /EHa /Od /Oi /W4 /wd4201 /wd4100 /wd4189 /wd4505 /DHANDMADE_INTERNAL=1 /DHANDMADE_SLOW=1 /Z7 
set CommonLinkerFlags= -incremental:no -opt:ref  user32.lib gdi32.lib winmm.lib

del *.pdb >NUL 2> NUL
cl /D_USRDLL /D_WINDLL %CommonCompilerFlags% ../handmade.cpp /Fmhandmade.map /LD /link /PDB:handmade_%random%.pdb /EXPORT:GameUpdateAndRender /EXPORT:GameGetSoundSamples -incremental:no
cl %CommonCompilerFlags% ../win32_handmade.cpp /Fmwin32_handmade.map /link %CommonLinkerFlags%
popd