# Onboarding process

It will help if you compile (run `make -j4') at each of these steps.

- [ ] Create a new branch named `onboarding-yourname`
- [ ] copy the `chibios-example` folder to a folder named `onboarding/yourname`
- [ ] Configure the GPIO to use ADC1 on a pin in the 64 pin package of the STM32
    - [ ] Read the datasheet to determine which pins can be used with ADC1
    - [ ] Read the reference manual to determine what modes to set the pins to to be useful
- [ ] Configure the ADC
    - [ ] Enable ADC1 in `cfg/halconf.h` and in `cfg/mcuconf.h`
    - [ ] call adcInit 
    - [ ] call adcStart with no configuration
    - [ ] read the STM32 reference manual and figure out how to configure the channel you need
        - sections 15.3.11 and 15.5.10 are relevant here
- [ ] Read the ADC values
    - [ ] In the main loop (or a new thread if you're feeling adventurous), read the ADC every second, and write those values out to UART.
