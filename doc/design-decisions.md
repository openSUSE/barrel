
Design Decisions
================


Cooperate with Existing Tools
-----------------------------

It is possible to use barrel along with other storage tools,
e.g. parted, mdadm, lvm and cryptsetup. The user can switch between
those tools whenever appropriate.

So barrel does not keep track of the system state is special
configuration files. Instead everything is probed from the system.

There might only be configuration files for default values, e.g. LVM
extent size, RAID metadata version, and to store pools.


Detect Errors Early
-------------------

In general mistakes should be detected as early as possible.

E.g. calling 'barrel create raid1 ... ext5 --path /test' reports that
ext5 is unknown before creating the RAID, even before probing the
system. On the other hand if the mount point /test already exists the
error is reported after probing but before creating the RAID.

