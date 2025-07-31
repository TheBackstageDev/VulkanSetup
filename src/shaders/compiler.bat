@echo off
setlocal enabledelayedexpansion

set SHADER_DIR=%cd%

set OUTPUT_DIR=%SHADER_DIR%

set GLSLANG_VALIDATOR=glslangValidator

if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)
for /r "%SHADER_DIR%" %%f in (*.vert) do (
    set SHADER_FILE=%%f
    set FILE_NAME=%%~nf
    set FILE_EXT=%%~xf

    set OUTPUT_FILE=%OUTPUT_DIR%\!FILE_NAME!!FILE_EXT!.spv

    echo Compiling !SHADER_FILE! to !OUTPUT_FILE!
    %GLSLANG_VALIDATOR% -V !SHADER_FILE! -o !OUTPUT_FILE!
    if !ERRORLEVEL! NEQ 0 (
        echo Error compiling !SHADER_FILE!
        exit /b 1
    )
)

for /r "%SHADER_DIR%" %%f in (*.frag) do (
    set SHADER_FILE=%%f
    set FILE_NAME=%%~nf
    set FILE_EXT=%%~xf

    set OUTPUT_FILE=%OUTPUT_DIR%\!FILE_NAME!!FILE_EXT!.spv

    echo Compiling !SHADER_FILE! to !OUTPUT_FILE!
    %GLSLANG_VALIDATOR% -V !SHADER_FILE! -o !OUTPUT_FILE!
    if !ERRORLEVEL! NEQ 0 (
        echo Error compiling !SHADER_FILE!
        exit /b 1
    )
)

echo All shaders compiled successfully!