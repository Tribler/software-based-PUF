### Update October 20 2025
The amount of time I have been able to devote to this project has been limited since May due to a family medical 
issue.  I'm posting the updates even though they are incomplete.  The latest design replaces TXB0106 with (3) 
74LXC2T45 voltage translators to achieve true Vcc isolation.  LVC translators were used in the prototype for testing 
but LXC are drop in replacements with some advantages.  Testing shows elimination of back-powering present with TXB0106.

The addition of a 3.3V onboard regulator has solved the reboot issue.  The Arduino 3.3V dedicated rail is unable to 
supply enough power but an AMS1117 or AP2112 5V --> 3.3V LDO should work since an AME1117 harvested from a junk board 
works well.  An LDO with an on/off switch would be ideal because the board initialization lags when the Arduino is 
powered and the SRAM is briefly powered with unconfigured I/Os. 

A TPS22917 switch is added to control Vcca supply, which in turn controls Vcc disconnect.  It's added on the schematic 
and board files but I didn't have one for testing.  Support has already been added in pufboard.  The schematic will 
be added soon.  It isn't final but it is fairly close.

[102025 figures](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/102025_update)


### Update May 2025 - Lessons learned
Second PCB revision has revealed some minor flaws plus an unexpected voltage transient during fast ramp that will be 
addressed for the next board revision (hopefully release).  Board fabrication was severely delayed and partly 
responsible for the slow progress.  A photo of the actual pcb_r2 board has been included (apologies for the poor 
quality) and is more representative of what to expect for release.  The Adafruit DAC and SD card breakouts plug into 
board mounted headers for simplicity.  The PCB layout may eventually be altered to produce an Arduino "hat" to 
completely eliminate hookup wires and errors.

The reboot issue on fast voltage ramp is resolved by using a dedicated power supply.  I dislike this constraint but 
will deal with it for now.

Because the SRAM is subject to unconventional operation, some datasheet recommendations and example circuit 
diagrams are effectively "wrong".  Following typical practices resulted in minor issues that are easily fixed - 
"wrong" pull-ups and a software issue improperly manipulating the CS line (original implementation is correct).

For revision r3, a load switch or MOSFETs will be added on SPI lines between SRAM and voltage level translator to 
prevent backpowering when SRAM is powered off.  An alternative voltage translator instead of TXB0106 may be used.

A diagnosis for the nature of the voltage transient when executing fast voltage ramp is needed.  It is unclear if it is 
due to inrush current, SRAM internals, etc.  The 23K640 datasheet specifies that Vdr (Ram Data Retention Voltage) is 
1.2 V, so cell data may be unaffected.  A quick test hack adding a 10 nF capacitor to the SRAM supply pin did not 
resolve the transient and raised ramp time to ~40 ns as expected due to charging and is not a solution.  A 
pre-charged capacitor/bank behind a gate might need to be added.  Other than the transient, the ramp waveform looks 
good.

[pcb_r2 figures](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/)

### Update February 2025

PCB for new bit selection method is nearing completion.  A render of the first revision PCB has been added to 
figures.  Initial prototype required a hardware fix to be usable and has been removed.  Testing performed on a 
modified prototype board show it works well but issues arose:

1. Unlike 23LC1024 which is 5V capable, 23K640 (3.6V max) needs a level shifter.  A TXB has been added for on board 
   level shifting.
2. Executing the fast ramp causes the Arduino to reboot, possibly due to voltage sag, but this needs to be debugged.
3. Fast ramp waveform is probably already acceptable but can benefit from some tuning of R6.  This will make more 
   sense when the schematic is released.

Although it may appear a little "cart before the horse", it is necessary to build the hardware first to replicate and 
incorporate the research into the software-based-PUF.

### Fast ramp circuit

Preliminary figures have been included showing a very unrefined fast ramp prototype circuit.  Due to the fast rise 
time required by the new bit selection method, an additional small breakout board with a full ground plane will be 
necessary.  Breadboarding the circuit will yield poor performance and inconsistent results due to unavoidable 
parasitic inductance and capacitance.  Schematic and Gerber files will be added to the project repo upon 
finalization.  Small production runs of populated finished boards can be offered if users are interested and should 
be inexpensive.

There are several suitable methods to achieve a rise time < 10ns but using a Nexperia 74LVC1G125/Q100 provides good 
performance at minimal added cost and complexity.  The Nexperia IC is capable of 32mA output drive.  The Microchip 
23K640I/SN SRAM has a maximum current draw of 10mA, easily handled by the 74LVC1G125.  The Nexperia IC employs a 
Schmitt trigger, which is beneficial in compensating for the Arduino's slow rise time.

The fast ramp circuit works by driving the 74LVC1G125 LOGIC input pin to the desired Vout.  Though intended for fast 
logic buffering, the rise time and high output drive allow it to be used as a fast power switch.  Supply and logic 
inputs are driven at 3.3V with the OE (output enable) pin initially set HIGH for the high impedance OFF state, 
regardless of the voltage on input pin A.  The OE pin is set LOW, removing the latch (ON state) to power the SRAM 
under a fast ramp condition.  SRAM can be powered off by setting HI-Z again (set OE pin HIGH).

Figures show a voltage ramp < 10ns.  The 23K640 Vcc and Gnd were connected for this test with the SPI lines left 
floating but this should not affect the ramp.  A slight amount of ringing is observed but the prototype lacks 
appropriately sized and placed decoupling capacitors on supply and logic lines.

[fast ramp waveform](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/74LVC1G125-20ns-div.png)

[fast ramp circuit](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/74LVC1G125-circuit.jpg)

TODO: test viability of socketed PDIP (23K640P) instead of the SOIC-8 23K640I/SN.  Parasitic inductance and 
capacitance may prevent usage.

Other options were considered, including FET based switching (specifically - EPC GaNFET solution).  Extreme low rise 
times (1 to 2 ns) are likely achievable with such a solution, but it is unclear if it would affect the outcome. Such 
a solution also adds complexity and cost since MOSFETs and GaNFETs require gate drivers.



### Slow ramp circuit

Slow voltage ramp can be achieved using an inexpensive DAC (Adafruit MCP4725 breakout board 12 bit DAC).

[slow ramp waveform](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/MCP4725-1s-ramp.png)

[slow ramp circuit](https://github.com/Tribler/software-based-PUF/tree/master/Source%20Code/enrollment%20and%20testing/slave-23k640-experimental/figures/MCP4725-ramp-dac.jpg)