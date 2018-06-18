# OTP for RIOT OS

Clone the repository and change directory

    git clone https://github.com/Agile-IoT/iotp-riot.git && cd iotp-riot && git clone https://github.com/RIOT-OS/RIOT.git

In the Makefile, change the device you want to run the code on, according to [RIOT OS Tutorial](https://github.com/RIOT-OS/Tutorials/tree/master/task-01).

    #BOARD ?= native
    BOARD ?=samr21-xpro
    
Flash the board

    sudo make all term flash

## Generate OTP

Create an OTP with the following pattern

    geneid IK COUNTER

IK = identity key, COUNTER = the counter for which the OTP should be generated.

For example:

    geneid e2e12c2281cdf3d350a34de4d5f56613 100000
    
The above example outputs

    INFO # Init Key Data: e2 e1 2c 22 81 cd f3 d3 50 a3 4d e4 d5 f5 66 13 
    INFO # Temporary Key Data: 00 00 00 00 00 00 00 00 00 00 00 ff 00 00 00 01 
    INFO # Temporary Key: fe a6 5d ed 44 a4 4e 36 e7 01 d8 33 88 28 63 22 
    INFO # Ephermal Id Data: 00 00 00 00 00 00 00 00 00 00 00 00 00 01 86 a0 
    INFO # Ephermal Id: e7 25 72 e3 84 ba 63 6b f5 01 83 ae 06 c7 c5 8f 
    INFO # Generated OTP: e7 25 72 e3 84 ba 63 6b

    
## Write key to internal storage

Write a string (id key) to the internal storage (flash page number) with the following pattern

    write_ik string page
    
For example:

    write_ik e2e12c2281cdf3d350a34de4d5f56613 100

The above example outputs

    INFO # write_ik e2e12c2281cdf3d350a34de4d5f56613 100
    INFO # successfully erased page 100 (addr 0x6400)
    INFO # wrote local page to flash page 100 at addr 0x6400
    
## Read from internal storage  

Read the key from the internal storage with the following pattern

    read_ik page
    
For example:

    read_ik 100
    
The above example outputs

    INFO # e2e12c2281cdf3d350a34de4d5f56613

## Run periodic OTP generation  

Once a key is written to flash (on page X) you can run periodic one-time-password generation based on this key:

    run_otp page
    
For example:

    run_otp 100

    read_ik page
    
The above outputs the sequence:

```

# Key read in internal memory:
# e2e12c2281cdf3d350a34de4d5f56613
# Counter read in internal memory: 
# 0 
# Init Key Data: 15 14 59 55 54 91 15 15 54 95 51 15 15 15 55 45 
# Temporary Key Data: 00 00 00 00 00 00 00 00 00 00 00 ff 00 00 00 00 
# Temporary Key: 5f 18 ec e8 6e c7 f0 f6 d9 31 cb 35 72 5c 1c 61 
# Ephermal Id Data: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
# Ephermal Id: b5 94 53 c4 9b 21 2f 01 74 d6 9a 22 6e cd 7e 31 
# Generated OTP: b5 94 53 c4 9b 21 2f 01 
# 
# Key read in internal memory:
# e2e12c2281cdf3d350a34de4d5f56613
# Counter read in internal memory: 
# 1 
# Init Key Data: 15 14 59 55 54 91 15 15 54 95 51 15 15 15 55 45 
# Temporary Key Data: 00 00 00 00 00 00 00 00 00 00 00 ff 00 00 00 00 
# Temporary Key: 5f 18 ec e8 6e c7 f0 f6 d9 31 cb 35 72 5c 1c 61 
# Ephermal Id Data: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 
# Ephermal Id: 26 55 8e 29 96 6a 83 b3 b8 58 7f c8 70 c3 e1 a2 
# Generated OTP: 26 55 8e 29 96 6a 83 b3 
# ...
```
