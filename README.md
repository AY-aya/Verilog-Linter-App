# Verilog-Linter-App
![logo](https://github.com/user-attachments/assets/80ab8f46-8412-459f-8d38-550693b1e25f)

This app is a prototype for a Verilog Linter application, developed using c++ and Qt quick 6.7.2.
This was developed only for learning purposes, for a university course on Electronic Design Automation
# Table of Contents

[1.Features](#Features)

[2.How to setup](#How-to-setup)

[3.Usage Example](#Usage-Example)

# Features
this linter supports the following verilog RTL design rule checks:
              
    1. Arithmetic Overflow
    2. Uninitialised Register Usage
    3. Unreachable Code Blocks
    4. Unreachable FSM states
    5. Inferred latches
    6. Multi-driven bus conflict
    7. Non-Full/Parallel Case detection
  please note that this is a simple prototype so only certain syntax issues are detected.
   
# How to setup
the simplist method for trying this app is by running the console version on any c++ ide : Verilog_linter_console_app.cpp

We also developed a simple gui using qt quick, this can be used by adding the header, source, and qml files to a qt project.

# Usage Example
![WhatsApp Image 2024-12-21 at 21 59 55_c2764411](https://github.com/user-attachments/assets/dcea0169-6bc3-4923-ab17-4da52d3a9b61)
![WhatsApp Image 2024-12-21 at 21 59 55_3359f6fe](https://github.com/user-attachments/assets/58339d07-1e36-4676-85b6-41231c3bb838)
![WhatsApp Image 2024-12-21 at 21 59 55_f9d7cd4d](https://github.com/user-attachments/assets/d2e9832a-e6aa-46e0-8a1d-09f5954f7105)



