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
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xd4c620cd, __VMLINUX_SYMBOL_STR(i2c_del_driver) },
	{ 0xc07a52f3, __VMLINUX_SYMBOL_STR(i2c_unregister_device) },
	{ 0xf8136911, __VMLINUX_SYMBOL_STR(i2c_smbus_read_byte_data) },
	{ 0xb32a1e50, __VMLINUX_SYMBOL_STR(i2c_put_adapter) },
	{ 0x3a2bc3d7, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x628d4a85, __VMLINUX_SYMBOL_STR(i2c_new_device) },
	{ 0x46fd1098, __VMLINUX_SYMBOL_STR(i2c_get_adapter) },
	{ 0xea3b776f, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x30ccc052, __VMLINUX_SYMBOL_STR(i2c_master_send) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DE9D0CDCF95DC605E1A6836");
