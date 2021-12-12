# ReadMe

This repo is part of ECE202 project, this part of the smart camera module. Please see [ECE202Project](https://github.com/RX-0-95/ecem202a_project.git) for detail.

## Clone all submodule recursively 

                git clone --recurse-submodules https://github.com/RX-0-95/ecem202a_project.git

## Compile uf2 file for Raspberry Pi Pico

1. Intall tool chain follow the instruction here [Pico intro](https://shawnhymel.com/2096/how-to-set-up-raspberry-pi-pico-c-c-toolchain-on-windows-with-vs-code/)

2. Run comman to build the executable 

        # make sure you are in the root directory of the cam_prject folder
        mkdir build
        cmake .. 
        make -j 10 

If build success, there will be two uf2 file in build/person_detection

*  **person_detetection_baremetal.uf2** is the bare metal implementation of the project
*  **person_detetection.uf2**  is the RTOS implementation of the project (Recommand)

3. Load uf2 file to Pico follow the instruction in [Pico guid](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
