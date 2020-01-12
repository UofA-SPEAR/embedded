# Onboarding Process

## Steps

- [ ] Create a new branch called `onboarding-yourname`
- [ ] Copy the folder `chibios-example` into a new folder `onboarding/yourname`
- [ ] Run `make -j4` to ensure the project compiles
- [ ] Make the example read an ADC value, and then publish that to UART.
    - [ ] Modify `cfg/halconf.h` and `cfg/mcuconf.h` to enable ADC1
    - [ ] Set up the input pin for the appropriate ADC1 channel
    - [ ] Set up a recurring read from the ADC and push that to UART
- [ ] Create a pull request for this branch to `master`
    - This pull request will not be accepted, but I will give you feedback there.


## Tips and Tricks

Take a quick look at the GPIO section in the reference manual for information on how those work.
Same thing with the ADC.



Check the pinout section in the datasheet to find out which alternate mode to set the pins
to to use them with your selected peripheral.
