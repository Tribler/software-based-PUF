# strong-software-based-Physical-Unclonable-Function

## README
This repository is dedicated for master thesis project by Ade Setyawan Sajim, TU Delft. All Arduino code in this repository is developed using [platformio](https://platformio.org/platformio-ide). The type of the Arduino is Arduino Mega 2560. The python code is developed using [PyCharm IDE](https://www.jetbrains.com/pycharm/download/#section=mac).

### Abstract
SRAM PUF has a potential to become the main player in hardware security. Unfortunately, the currently available solution is usually locked to specific entities. Here, we initiate an open source project to develop software-based SRAM PUF technology using off-the-shelf SRAM.
We also present a testing result on two off-the-shelf SRAMs as SRAM PUF candidates; Microchip 23LC1024 and Cypress CY62256NLL.
Both are tested on the distribution of 0's and 1's in their cells, intra hamming distance, inter hamming distance, and the effect of voltage variation and time interval between enrollment
Testing on two bit-selection algorithms (data remanence analysis and neighbor analysis) are also performed.
Based on the testing results, we introduce a PUF enrollment scheme using data remanence analysis as the bit selection algorithm which will locate the location of the stable bits and SRAM Cypress CY62256NLL as the off-the-shelf SRAM.
In addition, an idea to create a strong PUF using SRAM is also proposed here. Using a collection of bits as a challenge, the stable bits are permutated among themselves to create a challenge which has a tremendous number of possibilities.
Furthermore, we also present a secure data protection and key storage scheme using SRAM PUF. The proposed scheme is influenced by multi-factor authentication. Using a combination of a PUF-generated key and user's password, a derived key is produced and utilized as the final key to protect user's data or/and user's key.

### Repository structure
There are two folder inside:
- Thesis Report: contains latex files required for the thesis report
- Source Code: source code for the thesis project.
  - enrollment: source code required for enrollment.
    - master-python: python code, act as a master for the enrollment scheme
    - slave-arduino: arduino code
  - key storage scheme: Arduino source code for the secure data protection and key storage scheme.
    - PUF-decrypt: Arduino code to decrypt the ciphertext using the proposed scheme
    - PUF-encrypt: Arduino code to encrypt user's key using the proposed scheme

### Enrollment Scheme
#### Enrollment steps:
1. Connect Arduino with SRAM Cypress 62256NLL and microSD
2. Connect Arduino to PC using USB-to-serial cable
3. Upload slave-arduino project to Arduino
4. Run MainCypress.py from PC. Make sure the serial name and port is correct.
5. MainCypress.py will locate the strong bits using data remanence analysis, then shuffle those strong bits to generate PUF challenge. Afterwards, the PUF challenge is send to Arduino to be stored in microSD. Later, it will ask Arduino to generate the helper data and stored it in microSD.
5. When finished, microSD will have "challenge.txt" which contains PUF challenge and "helper_data.txt" which filled with helper data.

### Secure Data Protection and Key Storage Scheme
#### Key storage steps:
1. Connect Arduino with SRAM Cypress 62256NLL and microSD.
2. Connect Arduino to PC using USB-to-serial cable.
3. Upload PUF-encrypt project to Arduino.
4. Run the Arduino program by opening the serial port from PC.
5. The program will generate the PUF-generated key.
6. Wait until it ask for user's password.
7. Enter user's password.
8. The program will generate the final key.
9. Wait until it ask for user's key.
10. Enter user's key. It has to be 256-bits (16 characters)
11. The program will encrypt the user's key. The ciphertext will be stored in "encrypted.txt" in microSD.

#### Key reconstruction steps:
1. Connect Arduino with SRAM Cypress 62256NLL and microSD.
2. Connect Arduino to PC using USB-to-serial cable.
3. Upload PUF-decrypt project to Arduino.
4. Run the Arduino program by opening the serial port from PC.
5. The program will generate the PUF-generated key.
6. Wait until it ask for user's password.
7. Enter user's password.
8. The program will generate the final key.
9. The program will decrypt the ciphertext which stored in "encrypted.txt" in microSD.
10. The reconstructed user's key will be printed to the screen.
