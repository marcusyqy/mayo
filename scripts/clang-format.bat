@echo off
set FILEMASK=*.c,*.cc,*.cpp,*.h,*.hh,*.hpp
    :: Formats zoo folder
    pushd %~dp0\..\zoo
    for /R %%f in (%FILEMASK%) do (
        echo "formatting - %%f"
        :: Run twice as some files (e.g. Client\game_sa\CCameraSA.h) require it (bug?)
        clang-format -i %%f
        clang-format -i %%f
    )
    popd
PAUSE
