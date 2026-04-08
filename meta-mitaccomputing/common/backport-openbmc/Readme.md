To reduce the cost of migration, the recipe of packages under this directory is highly recommended be migrate from the same commit number of OpenBMC.
However, there are exceptions some package might bring a lots of changes on dependencies required to be upgrade. 
In this circumstance, individual needs to be stay at specified version and we need to maintane a table to document these exceptions.

* OpenBMC Backport Baseline: Commit# e2ca5f1780333377f7e4a385be8cbb445d7e250c

* Exceptions
| Package 		| OpemBMC Commit Number | Note	|
| dbus-sensors_git.bb 	| #COMMMIT NUMBER 	| ...	|

