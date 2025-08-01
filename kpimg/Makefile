# ======================
# 工具链配置
# ======================
TARGET := kpimg
override TARGET_COMPILE := aarch64-none-elf-
CC      := $(TARGET_COMPILE)gcc
LD      := $(TARGET_COMPILE)ld
AS      := $(TARGET_COMPILE)as
OBJCOPY := $(TARGET_COMPILE)objcopy

# ======================
# 编译选项
# ======================
CFLAGS += -Wall -fno-builtin -std=gnu11 -nostdinc
CFLAGS += -Wno-unused-function -Wno-return-type -Wno-c23-extensions -Wno-uninitialized
CFLAGS += -fno-PIC -fno-asynchronous-unwind-tables -fno-stack-protector -fno-unwind-tables -fno-semantic-interposition -U_FORTIFY_SOURCE -fno-common

ifdef DEBUG
CFLAGS += -DDEBUG -DMAP_DEBUG
endif

# ======================
# 包含路径
# ======================
INCLUDE := -I. -Iinclude -Ipatch/include -Ilinux 
INCLUDE += -Ilinux/include -Ilinux/arch/arm64/include 
INCLUDE += -Ilinux/tools/arch/arm64/include -Ikernel-source

# ======================
# 源文件集合
# ======================
BASE_SRCS += base/setup.c 
BASE_SRCS += base/setup1.S
BASE_SRCS += base/cache.S
BASE_SRCS += base/tlsf.c
BASE_SRCS += base/start.c 
BASE_SRCS += base/map.c 
BASE_SRCS += base/map1.S 
BASE_SRCS += base/hook.c 
BASE_SRCS += base/fphook.c 
BASE_SRCS += base/hmem.c 
BASE_SRCS += base/predata.c 
BASE_SRCS += base/symbol.c 
BASE_SRCS += base/baselib.c 
BASE_SRCS += base/sha256.c 

BASE_SRCS += $(wildcard patch/*.c)
BASE_SRCS += $(wildcard patch/common/*.c)
BASE_SRCS += $(wildcard patch/module/*.c)
BASE_SRCS += $(wildcard patch/ksyms/*.c)

BASE_SRCS += patch/android/sepolicy_flags.c

BASE_SRCS += $(wildcard patch/sukisu/*.c)


SRCS += $(BASE_SRCS)
SRCS += $(LINUX_SRCS)

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

# ======================
# 构建目标
# ======================
all: $(TARGET)

$(TARGET): $(TARGET).elf
	$(OBJCOPY) -O binary -S $^ $@

$(TARGET).elf: $(OBJS)
	${LD} -nostdlib -static -no-pie -Tkpimg.lds -e start -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -O2 -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

# ======================
# 清理
# ======================
.PHONY: clean
clean:
	@rm -f $(TARGET).elf $(TARGET)
	@find . -name '*.o' -exec rm -f {} +