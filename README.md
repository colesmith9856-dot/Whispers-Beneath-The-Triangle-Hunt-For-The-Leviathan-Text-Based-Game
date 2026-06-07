# Whispers-Beneath-The-Triangle-Hunt-For-The-Leviathan-Text-Based-GameWhispers Beneath the Triangle
An STM32-powered text adventure game built in C and Assembly for the STM32L4 platform.

Overview:
Whispers Beneath the Triangle is a UART-based adventure game where players investigate a mysterious disappearance in the Bermuda Triangle. Starting aboard a submarine, players explore underwater ruins, battle ancient creatures, solve puzzles, and uncover dimensional anomalies hidden beneath the ocean. This game has 2 modes, easy and hard. Hard mode offers more complex boss and enemy mechanics.

Features:
STM32L4 microcontroller implementation
UART terminal interface (PuTTY/Tera Term compatible)
Character selection system
Mason (HK M27, M1 Garand)
Ivan (AK-105, ShAK-12)
Multiple combat encounters and boss fights
Status effects and upgrade system
Puzzle-solving mechanics
C and Assembly integration (Assembly for damage calculations)
Multiple environments including:
Ocean Floor
Crystal Pyramid
Mosasaur Arena
Crystal Spires
Leviathan Arena
Technologies Used:
C
ARM Assembly
STM32 HAL Drivers
UART Communication
DMA 
STM32CubeIDE

Hardware:
Designed for STM32L4 development boards using USART2 serial communication.

Running the Project:
Open the project in STM32CubeIDE.
Build the project.
Flash to a compatible STM32L4 board.
Connect to USART2 using PuTTY or another serial terminal.
Reset the board and begin playing.

Project Summary:
This project demonstrates:
Embedded systems programming
UART communication
Memory-constrained game design
C and Assembly integration
State machine design
Real-time user interaction on microcontrollers
