# OrangeS

先用bximage生成一个软盘镜像 （a.img）
```
bximage
# 1
# fd
# default...
```

用bximage生成一个硬盘镜像 （80m.img）
```
bximage
# 1
# hd
# flat
# 80
# 80m.img
```

硬盘分区 (80m.img)
```
/sbin/fdisk 80m.img
# x
# c
# 162
# h
# 16
# r
# n
# p
# 1
# default
# +20
# n
# e
# 2
# default
# default
# w
```


[Error!] mount: /mnt/floppy/: mount point does not exist. 
```
mkdir /mnt/floppy
```