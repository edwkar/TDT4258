#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xca3acb9, "struct_module" },
	{ 0x852abecf, "__request_region" },
	{ 0xa3c42407, "cdev_del" },
	{ 0x7229b3ee, "cdev_init" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xb407b205, "ioport_resource" },
	{ 0xdd132261, "printk" },
	{ 0x49b7b5b4, "cdev_add" },
	{ 0xef79ac56, "__release_region" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "FD881CB155E991AF3C84BE5");
