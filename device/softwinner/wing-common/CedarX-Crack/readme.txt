-----------------------------------------------------------------
Purpose of this patch
This patch is used for supporting AC3/DTS/RMVB decode on A20 platform.


-----------------------------------------------------------------
How to use this patch?
1. Make sure you have got the sdk for A20, and the version is 'exdroid4.2.2r1-a20-v1.2'
2. Unzip the package to android/device/softwinner/wing-common/, make sure the dll lib is as follows:
   'device/softwinner/wing-common/CedarX-Crack/xxx.so'
3. Copy lib to the product directory, add code to 'android/device/softwinner/wing-common/ProductCommon.mk' as forllows:
# cedar crack lib so
PRODUCT_COPY_FILES += \
	device/softwinner/wing-common/CedarX-Crack/libdemux_rmvb.so:system/lib/libdemux_rmvb.so \
	device/softwinner/wing-common/CedarX-Crack/librm.so:system/lib/librm.so \
	device/softwinner/wing-common/CedarX-Crack/libswa1.so:system/lib/libswa1.so \
	device/softwinner/wing-common/CedarX-Crack/libswa2.so:system/lib/libswa2.so

