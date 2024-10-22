
### Fast ramp circuit

Preliminary figures have been included showing a very unrefined fast ramp prototype circuit.  Due to the fast rise 
time required by the new bit selection method, an additional small breakout board with a full ground plane will be 
necessary.  Breadboarding the circuit will yield poor performance and inconsistent results due to unavoidable stray 
inductance and capacitance.  Schematic and Gerber files will be added to the project repo upon finalization.  Small 
production runs of populated finished boards can be offered if users are interested and should be inexpensive.

There are several suitable methods to achieve a rise time < 10ns but using a Nexperia 74LVC1G125/Q100 provides good 
performance at minimal added cost and complexity.  The Nexperia IC is capable of 24mA output drive, which is 
uncommon in this class.  The Microchip 23K640I/SN SRAM has a maximum current draw of 10mA, easily handled by the 
74LVC1G125.  The Nexperia IC employs a Schmitt trigger, which is beneficial in compensating for the Arduino's slow 
rise time.

The fast ramp circuit works by driving the 74LVC1G125 LOGIC input pin to the desired Vout.  Though intended for fast 
logic switching, the output drive is high enough to exploit as a fast power switch.  Supply and logic inputs are 
driven at 3.3V with the OE (output enable) pin initially set HIGH for the high impedence OFF state, regardless of 
the voltage on input pin A.  The OE pin is set LOW, removing the latch (ON state) to power the SRAM under a fast 
ramp condition.  SRAM can be powered off by setting HI-Z again (set OE pin HIGH).

Figures show a voltage ramp < 10ns.  The 23K640 Vcc and Gnd were connected for this test with the SPI lines left 
floating but this should not affect the ramp.  A slight amount of ringing is observed but the prototype lacks 
appropriately sized and placed decoupling capacitors on supply and logic lines.

[fast ramp waveform](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/74LVC1G125-20ns-div.png)

[fast ramp circuit](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/74LVC1G125-circuit.jpg)

TODO: test viability of socketed PDIP (23K640P) instead of the SOIC-8 23K640I/SN.  Stray inductance/capacitance may 
prevent usage.

Other options were considered, including FET based switching (specifically - EPC GaNFET solution).  Extreme low rise 
times (1 to 2 ns) are likely achievable with such a solution, but it is unclear if it would affect the outcome. Such 
a solution also adds complexity and cost since MOSFETs and GaNFETs require gate drivers.



### Slow ramp circuit

Slow voltage ramp can be achieved using an inexpensive DAC (Adafruit MCP4725 breakout board 12 bit DAC).

[slow ramp waveform](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/MCP4725-1s-ramp.png)

[slow ramp circuit](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/MCP4725-ramp-dac.jpg)