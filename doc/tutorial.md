
Tutorial
--------

Barrel is a tool to manage several storage technologies on Linux. It
supports file systems, RAIDs, partition tables, LVM volume groups and
logical volumes. In this tutorial we will show some basic things
barrel can do for you.

In our first example we create an XFS on /dev/sdb1:

~~~
# barrel create filesystem --type xfs /dev/sdb1
Probing... done
  Create xfs on /dev/sdb1 (16.00 GiB)
Commit changes? [y/n] y
~~~

This simply runs mkfs.xfs so is not very exiting. So let's tell barrel
to create a XFS on /dev/sdb and also give a mount point:

~~~
# barrel create xfs /dev/sdb --size 10g --path /test1
Probing... done
  Create partition /dev/sdb2 (10.00 GiB)
  Create xfs on /dev/sdb2 (10.00 GiB)
  Mount /dev/sdb2 (10.00 GiB) at /test1
  Add mount point /test1 of /dev/sdb2 (10.00 GiB) to /etc/fstab
Commit changes? [y/n] y
~~~

As you can see barrel creates a partition, then the XFS, mounts it and
also adds an entry in /etc/fstab. Doing that with parted, mkfs.xfs,
mount and vi will likely take much more time.

In case you have several disks another feature of barrel called pools
is helpful. Pools are a collection of devices, e.g. disks. When first
started barrel will create pools for you:

~~~
# barrel show pools
Probing... done
Name         │ Devices │       Size │    Used
─────────────┼─────────┼────────────┼────────
HDDs (512 B) │       4 │ 128.00 GiB │  20.31%
├─/dev/sdb   │         │  32.00 GiB │  81.25%
├─/dev/sdc   │         │  32.00 GiB │   0.00%
├─/dev/sdd   │         │  32.00 GiB │   0.00%
└─/dev/sde   │         │  32.00 GiB │   0.00%
SSDs (512 B) │       3 │  96.00 GiB │  33.33%
├─/dev/sda   │         │  32.00 GiB │ 100.00%
├─/dev/sdf   │         │  32.00 GiB │   0.00%
└─/dev/sdg   │         │  32.00 GiB │   0.00%
~~~

So far disks of a pool must already have a GPT. Now we can tell barrel
to create an XFS from a pool. In that case barrel will select the
disk:

~~~
# barrel create xfs --size 20g --pool "HDDs (512 B)" --path /test2
Probing... done
  Create partition /dev/sdc1 (20.00 GiB)
  Create xfs on /dev/sdc1 (20.00 GiB)
  Mount /dev/sdc1 (20.00 GiB) at /test2
  Add mount point /test2 of /dev/sdc1 (20.00 GiB) to /etc/fstab
Commit changes? [y/n] y
~~~

Let's look at the file systems we have on our system by now:

~~~
# barrel show filesystems
Probing... done
Type │ Label │ Device    │ Mount Point
─────┼───────┼───────────┼────────────
xfs  │       │ /dev/sdb1 │
xfs  │       │ /dev/sdb2 │ /test1
xfs  │       │ /dev/sdc1 │ /test2
ext4 │       │ /dev/sda2 │ /
swap │       │ /dev/sda3 │ swap
~~~

Barrel can also show information about disks, raids and LVM volume
groups.

You might have noticed that probing always takes a few seconds and
that can be annoying. This can be avoided by using the interactive
mode where probing happens only once and then many commands can be
run. The interactive mode is started when no command is provided on
the command line.

We will now create a RAID with an XFS on top:

~~~
# barrel
Probing... done
barrel> create raid1 --size 10g --devices 2 --pool "HDDs (512 B)" xfs --path /test3
  Create partition /dev/sdd1 (10.12 GiB)
  Set id of partition /dev/sdd1 to Linux RAID
  Create partition /dev/sde1 (10.12 GiB)
  Set id of partition /dev/sde1 to Linux RAID
  Create MD RAID1 /dev/md0 (10.00 GiB) from /dev/sdd1 (10.12 GiB) and /dev/sde1 (10.12 GiB)
  Create xfs on /dev/md0 (10.00 GiB)
  Mount /dev/md0 (10.00 GiB) at /test3
  Add mount point /test3 of /dev/md0 (10.00 GiB) to /etc/fstab
  Add /dev/md0 to /etc/mdadm.conf
~~~

First the RAID is created and that RAID is then used as the block
device for the XFS. This is implemented using a stack: The create raid
command places the RAID on the stack and the create xfs command takes
it from the stack. The stack is preserved over several commands so
that

~~~
barrel> create raid [...] xfs [...]
~~~

is equivalent to:

~~~
barrel> create raid [...]
barrel[1]> create xfs [...]
~~~

The "[1]" in the prompt indicates that one object is on the stack. The
stack can also be shown using the command "stack".

Now we can look at the result of creating the RAID and XFS:

~~~
barrel[1]> show raids
Name     │      Size │ Level │ Metadata │ Chunk Size │ Devices │ Usage │ Pool
─────────┼───────────┼───────┼──────────┼────────────┼─────────┼───────┼─────
/dev/md0 │ 10.12 GiB │ RAID1 │ 1.0      │            │ 2       │ xfs   │
~~~

In the interactive mode you can also undo previous commands using the
command "undo". But we are happy with the result and so commit the
changes to disk:

~~~
barrel[1]> commit
  Create partition /dev/sdd1 (10.12 GiB)
  Set id of partition /dev/sdd1 to Linux RAID
  Create partition /dev/sde1 (10.12 GiB)
  Set id of partition /dev/sde1 to Linux RAID
  Create MD RAID1 /dev/md0 (10.00 GiB) from /dev/sdd1 (10.12 GiB) and /dev/sde1 (10.12 GiB)
  Create xfs on /dev/md0 (10.00 GiB)
  Mount /dev/md0 (10.00 GiB) at /test3
  Add mount point /test3 of /dev/md0 (10.00 GiB) to /etc/fstab
  Add /dev/md0 to /etc/mdadm.conf
Commit changes? [y/n] y
~~~

Finally one example to create an LVM volume group with one logical
volume and an ext4 in one step:

~~~
barrel> create vg --name vg1 --size 10g --pool "SSDs (512 B)" lv --name lv1 --size 5g ext4 --path /test4
  Create partition /dev/sdf1 (10.00 GiB)
  Set id of partition /dev/sdf1 to Linux LVM
  Create physical volume on /dev/sdf1
  Create volume group vg1 (10.00 GiB) from /dev/sdf1 (10.00 GiB)
  Create logical volume lv1 (5.00 GiB) on volume group vg1
  Create ext4 on /dev/vg1/lv1 (5.00 GiB)
  Mount /dev/vg1/lv1 (5.00 GiB) at /test4
  Add mount point /test4 of /dev/vg1/lv1 (5.00 GiB) to /etc/fstab
~~~

So, that is all for the tutorial. More information is available in the
barrel man-page.
