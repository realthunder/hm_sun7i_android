# wing fpga product config

$(call inherit-product, device/softwinner/wing-common/ProductCommon.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/sugar-cubieaio/overlay

PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/modules/modules/nand.ko:root/nand.ko \
	device/softwinner/sugar-cubieaio/modules/modules/sun7i-ir.ko:root/sun7i-ir.ko \
	#device/softwinner/sugar-cubieaio/modules/modules/disp.ko:root/disp.ko \
	#device/softwinner/sugar-cubieaio/modules/modules/lcd.ko:root/lcd.ko \
	#device/softwinner/sugar-cubieaio/modules/modules/hdmi.ko:root/hdmi.ko \

PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/kernel:kernel \
	device/softwinner/sugar-cubieaio/recovery.fstab:recovery.fstab \
	frameworks/base/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \

#add by Andy
PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/geek.sh:system/bin/geek.sh

#PRODUCT_COPY_FILES += \
         hardware/ril/reference-ril/init.gprs-pppd:system/etc/init.gprs-pppd:system \
         hardware/ril/reference-ril/ip-down:system/etc/ppp/ip-down  \
         hardware/ril/reference-ril/ip-up:system/etc/ppp/ip-up  \

PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/ueventd.sun7i.rc:root/ueventd.sun7i.rc \
	device/softwinner/sugar-cubieaio/init.sun7i.rc:root/init.sun7i.rc \
	device/softwinner/sugar-cubieaio/init.sun7i.usb.rc:root/init.sun7i.usb.rc \
	device/softwinner/sugar-cubieaio/init/init.sda.rc:root/init.sda.rc \
	device/softwinner/sugar-cubieaio/init/init.sdb.rc:root/init.sdb.rc \
	device/softwinner/sugar-cubieaio/init/init.sdc.rc:root/init.sdc.rc \
	device/softwinner/sugar-cubieaio/init/init.sdd.rc:root/init.sdd.rc \
	device/softwinner/sugar-cubieaio/init/init.sde.rc:root/init.sde.rc \
	device/softwinner/sugar-cubieaio/init.recovery.sun7i.rc:root/init.recovery.sun7i.rc \
	device/softwinner/sugar-cubieaio/needfix.rle:root/needfix.rle \
	device/softwinner/sugar-cubieaio/camera.cfg:system/etc/camera.cfg \
	device/softwinner/sugar-cubieaio/media_profiles.xml:system/etc/media_profiles.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml

#input device config
PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/sw-keyboard.kl:system/usr/keylayout/sw-keyboard.kl \
	device/softwinner/sugar-cubieaio/sun7i-ir.kl:system/usr/keylayout/sun7i-ir.kl \
	device/softwinner/sugar-cubieaio/tp.idc:system/usr/idc/tp.idc \
	device/softwinner/sugar-cubieaio/gsensor.cfg:system/usr/gsensor.cfg

PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/initlogo.rle:root/initlogo.rle

PRODUCT_COPY_FILES += \
	device/softwinner/sugar-cubieaio/vold.fstab:system/etc/vold.fstab
	
PRODUCT_PACKAGES += \
	Phone \
	TvdLauncher \
	Bluetooth
#	TSCalibration2

#Test APK
PRODUCT_PACKAGES += \
        SoundRecorder

# wifi & bt config file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    system/bluetooth/data/main.nonsmartphone.conf:system/etc/bluetooth/main.conf

# rtl8723as bt fw and config
#PRODUCT_COPY_FILES += \
#	device/softwinner/wing-common/hardware/realtek/bluetooth/rtl8723as/rlt8723a_chip_b_cut_bt40_fw.bin:system/etc/rlt8723a_chip_b_cut_bt40_fw.bin \
#	device/softwinner/wing-common/hardware/realtek/bluetooth/rtl8723as/rtk8723_bt_config:system/etc/rtk8723_bt_config

# bcm40181 sdio wifi fw and nvram
#PRODUCT_COPY_FILES += \
#	hardware/broadcom/wlan/firmware/bcm40181/fw_bcm40181a2_p2p.bin:system/vendor/modules/fw_bcm40181a2_p2p.bin \
#	hardware/broadcom/wlan/firmware/bcm40181/fw_bcm40181a2_apsta.bin:system/vendor/modules/fw_bcm40181a2_apsta.bin \
#	hardware/broadcom/wlan/firmware/bcm40181/fw_bcm40181a2.bin:system/vendor/modules/fw_bcm40181a2.bin \
#	hardware/broadcom/wlan/firmware/bcm40181/bcm40181_nvram.txt:system/vendor/modules/bcm40181_nvram.txt

# bcm40183 sdio wifi fw and nvram
#PRODUCT_COPY_FILES += \
#	hardware/broadcom/wlan/firmware/bcm40183/fw_bcm40183b2_p2p.bin:system/vendor/modules/fw_bcm40183b2_p2p.bin \
#	hardware/broadcom/wlan/firmware/bcm40183/fw_bcm40183b2_apsta.bin:system/vendor/modules/fw_bcm40183b2_apsta.bin \
#	hardware/broadcom/wlan/firmware/bcm40183/fw_bcm40183b2.bin:system/vendor/modules/fw_bcm40183b2.bin \
#	hardware/broadcom/wlan/firmware/bcm40183/bcm40183_nvram.txt:system/vendor/modules/bcm40183_nvram.txt

# ap6210 sdio wifi fw and nvram
PRODUCT_COPY_FILES += \
	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2.bin:system/vendor/modules/fw_bcm40181a2.bin \
	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2_apsta.bin:system/vendor/modules/fw_bcm40181a2_apsta.bin \
	hardware/broadcom/wlan/firmware/ap6210/fw_bcm40181a2_p2p.bin:system/vendor/modules/fw_bcm40181a2_p2p.bin \
	hardware/broadcom/wlan/firmware/ap6210/nvram_ap6210.txt:system/vendor/modules/nvram_ap6210.txt \
	hardware/broadcom/wlan/firmware/ap6210/bcm20710a1.hcd:system/vendor/modules/bcm20710a1.hcd \
	hardware/broadcom/wlan/firmware/ap6210/bd_addr.txt:system/etc/firmware/bd_addr.txt

	
PRODUCT_PROPERTY_OVERRIDES += \
	dalvik.vm.heapsize=256m \
	dalvik.vm.heapstartsize=8m \
	dalvik.vm.heapgrowthlimit=96m \
	dalvik.vm.heaptargetutilization=0.75 \
	dalvik.vm.heapminfree=2m \
	dalvik.vm.heapmaxfree=8m \
	persist.sys.usb.config=mass_storage,adb \
	ril.sim.absent.do=close-antenna \
	ril.sim.pin.retry.numbers=5 \
	ril.sim.puk.retry.numbers=5 \
	ro.ril.ecclist=110,119,120,112,114,911 \
	rw.talkingstandby.enabled=1 \
	ro.sw.embeded.telephony=true \
	ro.property.tabletUI=true \
	ro.udisk.lable=sugar \
	ro.product.firmware=v3.0 \
	ro.sw.defaultlauncherpackage=com.softwinner.launcher \
	ro.sw.defaultlauncherclass=com.softwinner.launcher.Launcher \
	audio.output.active=AUDIO_CODEC \
	audio.input.active=AUDIO_CODEC \
	ro.audio.multi.output=true \
	ro.sw.directlypoweroff=true \
	ro.softmouse.left.code=71 \
    ro.softmouse.right.code=21 \
    ro.softmouse.top.code=70 \
    ro.softmouse.bottom.code=22 \
    ro.softmouse.leftbtn.code=2 \
    ro.softmouse.midbtn.code=-1 \
    ro.softmouse.rightbtn.code=-1 \
    ro.sw.shortpressleadshut=false \
    ro.sw.videotrimming=1 \
    ro.sw.usedHardwareMouse=false\
    persist.sys.device_name = MiniMax \
    sys.mdnsd.enable=true

$(call inherit-product-if-exists, device/softwinner/sugar-cubieaio/modules/modules.mk)

PRODUCT_CHARACTERISTICS := manu

# Overrides
PRODUCT_BRAND  := einstein
PRODUCT_NAME   := sugar_cubieaio
PRODUCT_DEVICE := sugar-cubieaio
PRODUCT_MODEL  := softcubie

