@echo off
set FILEMASK=*.c,*.cc,*.cpp,*.h,*.hh,*.hpp
set FILEPATHS=zoo,mods,sandbox
    :: Formats zoo folder
    for %%p in (%FILEPATHS%) do (
        pushd %~dp0\..\%%p
        for /R %%f in (%FILEMASK%) do (
            echo "formatting - %%f"
            clang-format -i %%f
            clang-format -i %%f
        )
        popd
    )
PAUSE
