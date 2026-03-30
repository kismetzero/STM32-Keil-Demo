@echo off
setlocal enabledelayedexpansion

:: ================= 配置区域 =================
:: 源文件夹路径 (包含 abc 开头文件的文件夹)
set "SourceDir=C:\PF\Project\KeilProjects\STM32-Keil-Demo\STM32F103C8T6-DEMO-RE\System\i2c_bus"

:: 目标文件夹路径 (存放符号连接的文件夹)
set "TargetDir=C:\PF\Project\KeilProjects\STM32-Keil-Demo\STM32F103C8T6-DEMO-RE\System"

:: 文件名前缀匹配规则
set "FilePrefix=i2c"
:: ===========================================

:: 检查源文件夹是否存在
if not exist "%SourceDir%" (
    echo [错误] 源文件夹不存在: %SourceDir%
    pause
    exit /b 1
)

:: 如果目标文件夹不存在，则创建它
if not exist "%TargetDir%" (
    echo [信息] 目标文件夹不存在，正在创建: %TargetDir%
    mkdir "%TargetDir%"
    if errorlevel 1 (
        echo [错误] 无法创建目标文件夹，请检查权限。
        pause
        exit /b 1
    )
)

echo [开始] 正在从 "%SourceDir%" 查找以 "%FilePrefix%" 开头的文件...
echo -----------------------------------------------------------

set "Count=0"

:: 遍历源目录下的文件
:: /a:-d 表示只匹配文件，排除目录
:: %%i 代表完整路径，%%~ni 代表文件名，%%~xi 代表扩展名
for %%i in ("%SourceDir%\%FilePrefix%*") do (
    :: 再次确认是文件而不是文件夹 (防止有名为 abc 的文件夹被误判)
    if not exist "%%i\" (
        set "FileName=%%~nxi"
        set "LinkPath=%TargetDir%\!FileName!"
        
        :: 检查目标位置是否已存在同名文件或链接
        if exist "!LinkPath!" (
            echo [跳过] 已存在: !FileName!
        ) else (
            echo 正在创建"!FileName!"
            :: 创建文件符号连接
            mklink "!LinkPath!" "%%i"
            if errorlevel 1 (
                echo [失败] 无法创建链接: !FileName! （可能需要管理员权限）
            ) else (
                echo [成功] 已创建: !FileName!
                set /a Count+=1
            )
        )
    )
)

echo -----------------------------------------------------------
echo [完成] 总共创建了 %Count% 个符号连接。
pause