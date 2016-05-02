install -p -m 644 8192cu.ko /lib/modules/$(uname -r)/kernel/drivers/net/wireless/
depmod -a $(uname -r) 
