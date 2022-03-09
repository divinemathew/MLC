#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa1f0b564, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0xa4f6549c, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xadfa8cdf, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x6dfc1316, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xd4c620cd, __VMLINUX_SYMBOL_STR(i2c_del_driver) },
	{ 0xc07a52f3, __VMLINUX_SYMBOL_STR(i2c_unregister_device) },
	{ 0xb32a1e50, __VMLINUX_SYMBOL_STR(i2c_put_adapter) },
	{ 0x3a2bc3d7, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x628d4a85, __VMLINUX_SYMBOL_STR(i2c_new_device) },
	{ 0x46fd1098, __VMLINUX_SYMBOL_STR(i2c_get_adapter) },
	{ 0xea3b776f, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0xb81960ca, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xf8136911, __VMLINUX_SYMBOL_STR(i2c_smbus_read_byte_data) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x34b5b9b9, __VMLINUX_SYMBOL_STR(i2c_smbus_write_byte_data) },
	{ 0x91225ac9, __VMLINUX_SYMBOL_STR(i2c_smbus_read_word_data) },
	{ 0xf7c34cfd, __VMLINUX_SYMBOL_STR(i2c_smbus_write_block_data) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "217135F899B5D46E51418D3");
