#=============================================================================
#=============================================================================


obj-y   += camera_legacy/
obj-y   += connectivity/
ifneq ($(strip $(OBB_SEPARATE)),true)
obj-y   += ext_modem/
endif
obj-y += chr/
obj-y   += sensor/
#remove for debug
obj-y   += inputhub/
obj-y   += touthscreen/
obj-$(CONFIG_MAXQ616_POWER_SWITCH) += irda/
#obj-${CONFIG_HUAWEI_DSM} += dsm/
obj-$(CONFIG_LOGGER_EXTEND) += log/
obj-y   += audio/
obj-y   += nfc/
obj-y   += usb/
obj-$(CONFIG_ANTENNA_FPGA) += antenna_fpga/
obj-y   += power/
obj-y	+= storage/
obj-y   += chipverification/
obj-$(CONFIG_HUAWEI_DSM) += recovery/
obj-$(CONFIG_KRETPROBES) += rphone/
