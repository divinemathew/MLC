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
	{ 0xd4c620cd, __VMLINUX_SYMBOL_STR(i2c_del_driver) },
	{ 0xc07a52f3, __VMLINUX_SYMBOL_STR(i2c_unregister_device) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xb32a1e50, __VMLINUX_SYMBOL_STR(i2c_put_adapter) },
	{ 0x3a2bc3d7, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x628d4a85, __VMLINUX_SYMBOL_STR(i2c_new_device) },
	{ 0x46fd1098, __VMLINUX_SYMBOL_STR(i2c_get_adapter) },
	{ 0x4d4f99bd, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0x6a0111cd, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x12da5bb2, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xea3b776f, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0x67d8bf40, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0xc89ac6fb, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0x2f9a685e, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:ETX_OLED");

MODULE_INFO(srcversion, "C5681281267B0A29EA6399C");
