# pyrix64
opensource simple cpu (hoping to achive a 64bit cpu..)

## gpr
- `r0` zero register
- `r31` return adress register
- `r1`–`r30` normal gpr 64 bit
- `FP`, `PC` `STATUS` are separate 64 bit, no gpr
- `EPC`, `CAUSE` `TVEC` also seperate 64 bit registers

## Privilege levels
we have 4 privelege levels (pl)

| Level | Name | Purpose |
|---|---|---|
| PL0 | user | usermode |
| PL1 | driver | driver mode |
| PL2 | kernel | kernelmode |
| PL3 | secure monitor | secure monitor mode? |

## Stack pointers
every pl has its own stack pointer

| Stack pointer | Privilege level | Purpose |
|---|---|---|
| SP0 | PL0 | user stack |
| SP1 | PL1 | driver stack |
| SP2 | PL2 | kernel stack |
| SP3 | PL3 | secure monitor stack |

## License
This project is licensed under the
GNU General Public License v3.0.
