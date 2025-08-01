#include <log.h>
#include <ksyms.h>
#include <kallsyms.h>
#include <hook.h>
#include <accctl.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/cred.h>
#include <linux/capability.h>
#include <syscall.h>
#include <module.h>
#include <predata.h>
#include <linux/string.h>

void print_bootlog()
{
    const char *log = get_boot_log();
    char buf[1024];
    int off = 0;
    char c;
    for (int i = 0; (c = log[i]); i++) {
        if (c == '\n') {
            buf[off++] = c;
            buf[off] = '\0';

            printk("KP %s", buf);
            off = 0;
        } else {
            buf[off++] = log[i];
        }
    }
}

void before_panic(hook_fargs12_t *args, void *udata)
{
    printk("==== Start KernelPatch for Kernel panic ====\n");
    print_bootlog();
    printk("==== End KernelPatch for Kernel panic ====\n");
}

void linux_misc_symbol_init();
void linux_libs_symbol_init();

int resolve_struct();
int task_observer();
int bypass_kcfi();
int bypass_selinux();
int resolve_pt_regs();
int supercall_install();
void module_init();
void syscall_init();
int kstorage_init();
int su_compat_init();

int android_user_init();
int android_sepolicy_flags_fix();

void init_patchsu();
void init_sukisu_ultra();

static void before_rest_init(hook_fargs4_t *args, void *udata)
{
    int rc = 0;
    log_boot("entering init ...\n");

    if ((rc = bypass_kcfi())) goto out;
    log_boot("bypass_kcfi done: %d\n", rc);

    if ((rc = resolve_struct())) goto out;
    log_boot("resolve_struct done: %d\n", rc);

    // if ((rc = bypass_selinux())) goto out;
    // log_boot("bypass_selinux done: %d\n", rc);

    if ((rc = task_observer())) goto out;
    log_boot("task_observer done: %d\n", rc);

    // 砍掉SuperCall
    // rc = supercall_install();
    // log_boot("supercall_install done: %d\n", rc);

    rc = kstorage_init();
    log_boot("kstorage_init done: %d\n", rc);

    // 砍掉AP的ROOT逻辑
    // rc = su_compat_init();
    // log_boot("su_compat_init done: %d\n", rc);

    // 自己的逻辑
    init_sukisu_ultra();

    rc = resolve_pt_regs();
    log_boot("resolve_pt_regs done: %d\n", rc);

    rc = android_sepolicy_flags_fix();
    log_boot("android_sepolicy_flags_fix done: %d\n", rc);

    // rc = android_user_init();
    // log_boot("android_user_init done: %d\n", rc);

out:
    return;
}

static int extra_event_pre_kernel_init(const patch_extra_item_t *extra, const char *args, const void *data, void *udata)
{
    if (extra->type == EXTRA_TYPE_KPM) {
        if (!strcmp(EXTRA_EVENT_PRE_KERNEL_INIT, extra->event) || !extra->event[0]) {
            int rc = load_module(data, extra->con_size, args, EXTRA_EVENT_PRE_KERNEL_INIT, 0);
            log_boot("load kpm: %s, rc: %d\n", extra->name, rc);
        }
    }
    return 0;
}

static void before_kernel_init(hook_fargs4_t *args, void *udata)
{
    log_boot("event: %s\n", EXTRA_EVENT_PRE_KERNEL_INIT);
    on_each_extra_item(extra_event_pre_kernel_init, 0);
}

static void after_kernel_init(hook_fargs4_t *args, void *udata)
{
    log_boot("event: %s\n", EXTRA_EVENT_POST_KERNEL_INIT);
}

int patch()
{
    linux_libs_symbol_init();
    linux_misc_symbol_init();
    module_init();
    syscall_init();

    hook_err_t rc = 0;

    unsigned long panic_addr = patch_config->panic;
    logkd("panic addr: %llx\n", panic_addr);
    if (panic_addr) {
        rc = hook_wrap12((void *)panic_addr, before_panic, 0, 0);
        log_boot("hook panic rc: %d\n", rc);
    }
    if (rc) return rc;

    // rest_init or cgroup_init
    unsigned long init_addr = patch_config->rest_init;
    if (!init_addr) init_addr = patch_config->cgroup_init;
    if (init_addr) {
        rc = hook_wrap4((void *)init_addr, before_rest_init, 0, (void *)init_addr);
        log_boot("hook rest_init rc: %d\n", rc);
    }
    if (rc) return rc;

    // kernel_init
    unsigned long kernel_init_addr = patch_config->kernel_init;
    if (kernel_init_addr) {
        rc = hook_wrap4((void *)kernel_init_addr, before_kernel_init, after_kernel_init, 0);
        log_boot("hook kernel_init rc: %d\n", rc);
    }

    return rc;
}
