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
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xf8136911, __VMLINUX_SYMBOL_STR(i2c_smbus_read_byte_data) },
	{ 0xb32a1e50, __VMLINUX_SYMBOL_STR(i2c_put_adapter) },
	{ 0x3a2bc3d7, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x628d4a85, __VMLINUX_SYMBOL_STR(i2c_new_device) },
	{ 0x46fd1098, __VMLINUX_SYMBOL_STR(i2c_get_adapter) },
	{ 0xea3b776f, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xe95b9b10, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0x4d4f99bd, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0x6a0111cd, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0x67d8bf40, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0xc89ac6fb, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0x2f9a685e, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x30ccc052, __VMLINUX_SYMBOL_STR(i2c_master_send) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DED54F0821577358FCDB32D");
