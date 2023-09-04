# SPDX-License-Identifier: GPL-2.0-only

obj-$(CONFIG_AXERA_DIAG_CORE)	+= diag-core.o
diag-core-objs = diag_core.o diag_netlink.o
diag-core-objs += diag_sysfs.o
diag-core-objs += diag_fault_inject.o
diag-core-$(CONFIG_DIAG_CORE_SELFTEST) += diag_selftest.o

obj-$(CONFIG_AXERA_DIAG_TEST) += diag_api_test.o
