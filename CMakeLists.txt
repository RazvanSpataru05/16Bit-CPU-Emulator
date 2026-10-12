cmake_minimum_required(VERSION 3.20)

project(CPU-Simulator-8bit
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Source files
# ============================================================

set(SOURCES
    src/Main.cpp

    src/Assembler/ISAEntry.cpp
    src/Assembler/Lexer.cpp
    src/Assembler/Parser.cpp
    src/Assembler/Assembler.cpp

    src/Core/ALU.cpp
    src/Core/CPU.cpp
    src/Core/Emulator.cpp
    src/Core/Keyboard.cpp
    src/Core/MemoryUnit.cpp
    src/Core/InterruptController.cpp
    src/Core/RegisterFile.cpp
    src/Core/StatusRegister.cpp
    src/Core/Terminal.cpp

    src/Debugger/Dissasembler.cpp

    src/IO/DataLoader.cpp

    src/UI/UIEditor.cpp
    src/UI/Application.cpp

    src/Utils/CharacterUtils.cpp
    src/Utils/StringUtils.cpp
    src/Utils/Logger.cpp
    src/Utils/UIUtils.cpp
)

set(HEADERS
    inc/Assembler/InstructionDef.h
    inc/Assembler/ISAEntry.h
    inc/Assembler/Lexer.h
    inc/Assembler/ParsedNumber.h
    inc/Assembler/Parser.h
    inc/Assembler/LabelInfo.h
    inc/Assembler/Token.h
    inc/Assembler/Statement.h
    inc/Assembler/AssemblerStage.h
    inc/Assembler/AssemblerError.h
    inc/Assembler/AssemblerWarning.h
    inc/Assembler/Assembler.h

    inc/Core/ALU.h
    inc/Core/CPU.h
    inc/Core/Emulator.h
    inc/Core/MemoryUnit.h
    inc/Core/MemoryCellType.h
    inc/Core/Keyboard.h
    inc/Core/InterruptController.h
    inc/Core/RegisterFile.h
    inc/Core/Result.h
    inc/Core/StatusRegister.h
    inc/Core/Terminal.h
    inc/Core/TerminalLine.h

    inc/Debugger/Dissasembler.h

    inc/IO/DataLoader.h
    inc/IO/AddressValue.h
    inc/IO/Program.h

    inc/UI/UIEditor.h
    inc/UI/UICommon.h
    inc/UI/InstructionTableInfo.h
    inc/UI/Application.h
    inc/UI/AppConfig.h
    inc/UI/AppDefaults.h

    inc/Utils/CharacterUtils.h
    inc/Utils/StringUtils.h
    inc/Utils/Logger.h
    inc/Utils/UIUtils.h
)

# ============================================================
# Third-party (ImGui)
# ============================================================

set(IMGUI_SOURCES
    lib/ImGui/imgui.cpp
    lib/ImGui/imgui_demo.cpp
    lib/ImGui/imgui_draw.cpp
    lib/ImGui/imgui_tables.cpp
    lib/ImGui/imgui_widgets.cpp
    lib/ImGui/imgui-SFML.cpp
)

# ============================================================
# Executable
# ============================================================

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${IMGUI_SOURCES}
)

# ============================================================
# Header / include directories
# ============================================================

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/Assembler
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/Debugger
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/UI

        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/ImGui
)

# ============================================================
# SFML
# ============================================================

set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${SFML_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${SFML_DIR}/sfml-graphics-d.lib
        ${SFML_DIR}/sfml-window-d.lib
        ${SFML_DIR}/sfml-system-d.lib
        opengl32
)

# ============================================================
# Solution Explorer folder structure
# ============================================================

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})
source_group("ImGui" FILES ${IMGUI_SOURCES})

# ============================================================
# MSVC configuration
# ============================================================

if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            /W3
            /permissive-
        )
endif()