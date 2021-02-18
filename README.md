# ODMB Development Software

UCSB software to test ODMB functionality, in the process of being updated for ODMB7/5 testing. Original from me11/odmbdev.

## Setup
This is an application of TriDAS, need to checkout and compile the [TriDAS](https://gitlab.cern.ch/CMS-Emu/CERN/TriDAS) first. 
Currently the **odmbdev** software only works with TriDAS relase EmuLib 14.7 or lower.

``` bash
cd TriDAS/emu
git clone https://github.com/odmb/odmbdev.git
cd odmbdev
./complie.sh
```

After it's setup, the next time it can be started by
``` bash
# cd TriDAS/emu/odmbdev
./scripts/startXXX.sh
```
