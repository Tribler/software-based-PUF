=====================================
software-based-PUF development branch
=====================================

NOTICE:
-------
This is a development branch for software-based-PUF project and is incomplete and untested. Much of the development 
is intended to simplify the laborious process involved with SRAM testing.  Many changes must be made simultaneously 
in a monolithic update to succeed.  It is quite a lot of additional code, but is well commented throughout.  
*NOTE - using Pycharm is highly recommended.

CHANGES:
--------

1. Created a sram identification scheme (utilizing some existing code). SRAM's are profiled and assigned an index.
   This is a better solution than binding an SRAM to an index vicariously through an Arduino unique id. SRAM chips
   lack burned in unique id's but we can use the PUF concept for SRAM identification.  *NOTE - identifying SRAM's
   using this process is time intensive and is only applied to single chip usage (MONO config). PARALLEL profiling
   uses Arduino unique id for binding, thus SRAM chips must remain paired to a specific Arduino.

   Assuming cell value probabilities P(0)=.5, P(1)=.5, identification is easily shown to be reliable by the
   cumulative distribution function (CDF) of a binomial distribution since allowing even a 10% error tolerance (90%
   matching cell values) over 4662 trials has a probability of occurring by chance alone of essentially zero. A
   not to exceed error of 10% is below 12.5%, which is roughly the maximum correctable error given 4662 cells and
   using commonly chosen values for Reed Solomon (if my math is correct).

2. 23LC1024 is now partially working (recorded strong_bit locations and values). Testing showed roughly 95% stable
   cells without reboot between GetStableBitsValue runs, but stable cells fell to only 80% when rebooting between
   GetStableBitValue runs, which is not usable. Chaining two 23LC1024 may be a solution to acquire enough stable
   bits or using an alternate method to induce metastability may also get this chip usable (possibly others).

   Key storage scheme is not possible since the project lacks the necessary libs for 23LC1024 support (bch, tools?).  
   sram-23lc1024/SRAM.cpp is incomplete. Need to add key storage scheme support. Weak 23LC1024 main.cpp random
   seed code(generated on Arduino) was improved greatly, but external seeding (optional) using host /dev/urandom
   isn't working yet.

TODO:
-----

1. For PUF-encrypt/src/main.cpp, fix seed value used by randomSeed() inside gen_IV(). Options include reading some
   random data in over a serial connection from the host, or write random data to the sd prior to key storage
   routine and retrieve for seed.

2. Refactor added commands (i.e. GET_UID, etc.) to be encapsulated by the class SerialPUF.

3. Test and add support for alternative SRAM modules.  Add 23LC1024 if viable.

4. Add support for auto voltage variation with the addition of a board such as TPS6381xEVM for voltage variation
   testing.