# MLC - Multicolor LED Controller 

Multicolor LED controller is POC designed and developed as part of DCT internal Training. This code is the software used to control a multicolor LED connected to FRDM 64 Development board at prescribed PWM Channels. All hardware mappings and code design documents are in the directory MLC/docs

To get the code, open terminal and copy paste the commands

sudo apt-get update
sudo apt-get install git
cd /home/Desktop
git clone https://github.com/divinemathew/MLC 

## Folder Structure

Documents             -     MLC/docs
Release Binary        -     MLC/binaries
Source Code           -     MLC/src


## Core of Software 
The main section of the code can be found in 
MLC/src/MLC-Controller/source/

## MLC/src/MLC-Controller/source/comm_handler/
Software code related with I2C transfer, data transfer between all tasks in the software etc done in this folder
MLC/src/MLC-Controller/source/comm_handler/comm_handler.c   - Communication Handler C File
MLC/src/MLC-Controller/source/comm_handler/comm_handler.h   - Communication Hanlder H File

## MLC/src/MLC-Controller/source/pattern/
All functions and implimentation of pattern execution (LED PATTERN EXECUTION) is done in the file pattern.c in this directory

## MLC/src/MLC-Controller/source/ui/
All functions and implemention of UI are done in the file ui_handler.c

## MLC/src/MLC-Controller/source/
main.c -> main entry point of the software program, all task creation and RTOS scheduling is done in this file
