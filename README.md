# the-black-box
The Black Box is a smart “energy aware” plug which requests carbon-dioxide intensity data from the web and compares it to the day before. If the current CO2 emissions are less than yesterday at the same hour, the power strip connects to the main grid, otherwise no current will flow through.
In order to reduce carbon-dioxide emissions we must rise awareness of the way energy is produced and delivered. How our daily routines would change if we were forced to reduce our energy consumption? How long will it take until we reach the zero and what will be next? 

The project has been developed during the 11th edition of PIF camp (27 July – 2 August 2025) in Soča, Slovenia

## BOM
+ ESP8266 development board
+ 5VDC 250V 10A single channel relay
+ WS2812B LED

### Wiring

![alt text](src/img/the-black-box_opened.jpg "The Black Box wiring")

Photo credits: Katja Goljat

The wiring is pretty simple, the relay control pin goes to D1 and the the LED data line to D4. 
Main grid lead goes to relay NO (normally open) terminal and the other end to COM.

Note: mostly relay modules are rated 10A and don't have a fuse box so it's recommended to add it to the circuit.

**Please be aware that** interfacing to the main grid power can be extremely dangerous so please don't do it until you really know how to operate it in safe conditions!

### References:
[Energy Awareness workshop by Berhard Rasinger](https://pif.camp/pifcamp-s11-e01-god-of-light-intro/)

[Electricity maps API documentation](https://portal.electricitymaps.com/developer-hub/api/getting-started)

