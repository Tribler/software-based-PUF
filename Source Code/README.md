
# Section 1: Testing off-the-shelf SRAM
## Section 1.1:  Retrieve SRAM's stable bits using data remanence algorithm
1. Connect Arduino with an SRAM. To connect SRAM, refer to fzz file inside the folder 'schematic'. For Microchip 23LC1024, refers to schematic file 23LC1024-time-interval.fzz. For Cypress CY62256NLL, refers to schematic file CY62256NLL-time-interval.fzz.
2. Connect Arduino to PC using USB-to-serial cable.
3. Upload 'slave-23lc1024' to Arduino if one want to test SRAM Microchip 23LC1024. Otherwise, upload 'slave-cy62256nll' if Cypress CY62256NLL is the SRAM that want to be tested.
4. Open 'master-testing' project. Run GetStableBits.py to retrieve stable bits locations using data remanence algorithm.
5. The stable bits location will be saved in 'strongbits.txt'

## Section 1.2:  Retrieve SRAM's stable bits value on the effect of voltage variation:
1. Connect Arduino with an SRAM and step-up-down converter. To connect an SRAM, refer to fzz file inside the folder 'schematic'. For Microchip 23LC1024, refers to schematic file 23LC1024-voltage.fzz. For Cypress CY62256NLL, refers to schematic file CY62256NLL-voltage.fzz.
2. Set the output of the step-up-down converter to the desired voltage.
3. Open 'master-testing' project. Open GetStableBitsValue.py, set 'index_SRAM' according to the index of the tested SRAM component, 'bits_filename' to the name of the file containing stable bits, saved_filename' to the desired stable-bits-value filename. Run the python file to retrieve the value of stable bits.
4. Repeat step 2 and 3 until there are enough stable bits value. Don't forget to rename the stable-bits-value filename every time step 3 is performed.

## Section 1.3:  Retrieve SRAM's stable bits value on the effect of time interval:
1. Connect Arduino with an SRAM. To connect an SRAM, refer to fzz file inside the folder 'schematic'. For Microchip 23LC1024, refers to schematic file 23LC1024-time-interval.fzz. For Cypress CY62256NLL, refers to schematic file CY62256NLL-time-interval.fzz.
2. Open 'master-testing' project. Open GetStableBitsValue.py, set 'index_SRAM' according to the index of the tested SRAM component, 'bits_filename' to the name of the file containing stable bits, 'saved_filename' to the desired stable-bits-value filename. Run the python file to retrieve the value of stable bits.
3. Repeat step 2 until there are enough stable bits value. Don't forget to wait until the desired time interval is completed first, and rename the stable-bits-value filename every time step 2 is performed.

## Section 1.4:  Calculate the stability of the off-the-shelf SRAM:
1. Open 'master-testing' project.
2. Run CalculateStability.py to check the intra hamming distance of the SRAMs.
3. If the maximum intra fractional hamming distance is less than 12.8% (maximum error rate that can be corrected by our key generation scheme), this SRAM is stable enough to be a PUF component in our key generation scheme.

## Section 1.5:  Calculate the uniqueness of the off-the-shelf SRAM:
1. Open 'master-testing' project.
2. Repeat section 1.2 and 1.3 for other SRAMs with similar type. Set 'bits_filename' to the same file for all SRAMs. For example, if there are 10 SRAMs, use stable bits locations from the first SRAM as the location of the value that will be retrieved on other SRAMs.
3. Run CalculateUniqueness.py to check the inter hamming distance of the SRAMs.
4. If the minimum inter fractional hamming distance is bigger than 12.8% (maximum error rate that can be corrected by our key generation scheme), this SRAM is unique enough to be a PUF component in our key generation scheme. Ideally, the preferred inter fractional hamming distance is bigger than 35%.

## Section 1.6:  Conclusion on testing:
If the SRAM is unique and stable (pass the test on Section 1.5 and Section 1.6), we can use this SRAM as a PUF component in our application.

# Section 2: Enrollment Scheme
1. Connect Arduino with SRAM Cypress CY62256NLL and microSD. Refer to 'CY62256NLL-enrollment-microSD.fzz' in folder 'schematic' for connecting these components.
2. Connect Arduino to PC using USB-to-serial cable.
3. Upload 'slave-cy62256nll' project to Arduino
4. Open 'master-enrollment'. Run MainEnrollment.py from PC. Make sure the serial name and port is correct.
5. MainEnrollment.py will locate the stable bits using data remanence analysis, then shuffle these stable bits to generate PUF challenge. Afterwards, the PUF challenge is send to Arduino to be stored in microSD. Later, it will ask Arduino to generate the helper data and stored it in microSD.
6. When finished, microSD will have "challenge.txt" which contains PUF challenge and "helper_data.txt" which filled with helper data.

# Section 3: Reconstruction of PUF-Generated Key
1. Connect Arduino with SRAM Cypress CY62256NLL and microSD. Refer to 'CY62256NLL-enrollment-microSD.fzz' in folder 'schematic' for connecting these components.
2. Connect Arduino to PC using USB-to-serial cable.
3. Upload 'reconstruction' project to Arduino.
4. Run the program by opening serial connection to Arduino.
5. The program will show the PUF-generated key.

# Section 4: Secure Data and Key Storage Scheme
## Section 4.1: Encrypt Using The Final Key
1. Perform the enrollment stage explained in Section 2.
2. Upload 'PUF-encrypt' project to Arduino.
3. Run the program by opening serial connection to Arduino.
4. The program will generate the PUF-generated key.
5. Wait until it ask for user password.
6. Enter user password.
7. The program will generate the final key.
8. Wait until it ask for user key.
9. Enter user key. It has to be 256-bits (16 characters)
10. The program will encrypt the user key. The ciphertext will be stored in "e.txt" in microSD.

## Section 4.2: Decrypt Using The Final Key
1. Ensure the encryption stage is already performed (explained in Section 4.1).
2. Upload 'PUF-decrypt' project to Arduino.
3. Run the program by opening serial connection to Arduino.
4. The program will generate the PUF-generated key.
5. Wait until it ask for user password.
6. Enter user password.
7. The program will generate the final key.
8. The program will decrypt the user key which stored in "e.txt" in microSD.
10. The reconstructed user key will be printed to the screen.
