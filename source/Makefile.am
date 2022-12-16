#libxbase_la_SOURCES= b4node.c c4baspas.c c4blast.c c4buffer.c c4com.c c4com2.c c4send.c c4set.c  c4thread.c  c4util.c d4info.c e4event.c f4wstr.c  i4conv.c  i4thread.c l4mutex.c r4regs.h

SOURCES=  b4block.c  o4opt.c m4create.c m4file.c m4map.c m4memo.c m4memory.c
SOURCES+= c4.c  c4bcd.c  c4code.c   c4const.c c4hook.c  c4trans.c
SOURCES+= d4append.c d4bottom.c d4close.c d4create.c d4data.c d4date.c d4extra.c d4field.c
SOURCES+= d4file.c d4flush.c d4fresh.c d4go.c d4index.c  d4inline.c d4lock.c d4modify.c
SOURCES+= d4open.c d4opt.c d4pack.c d4positi.c d4remove.c d4seek.c d4skip.c d4tag.c d4top.c
SOURCES+= d4unlock.c d4write.c d4zap.c df4lock.c df4unlok.c
SOURCES+= e4calc.c e4calc_2.c e4error.c e4expr.c  e4functi.c e4not_s.c e4parse.c e4string.c
SOURCES+= f4ass_f.c f4char.c f4close.c f4create.c f4double.c f4field.c f4file.c f4filese.c
SOURCES+= f4flag.c f4flush.c f4info.c f4int.c f4lock.c f4long.c f4memo.c f4open.c f4opt.c f4ptr.c f4str.c f4temp.c
SOURCES+= f4true.c f4write.c
SOURCES+= i4add.c i4addtag.c i4check.c  i4create.c i4dump.c i4index.c i4info.c i4init.c i4key.c i4lock.c
SOURCES+= i4ntag.c i4positi.c i4remove.c i4tag.c
SOURCES+= l4link.c l4lock_c.c m4check.c
SOURCES+= r4area.c r4bit.c r4drive2.c r4driver.c r4group.c r4log.c r4obj.c r4reinde.c r4reindx.c
SOURCES+= r4relate.c r4report.c r4save.c r4save_2.c r4style.c r4total.c
SOURCES+= s4init.c s4initfr.c s4next.c s4quick.c s4sort.c s4signal.c sema4.c u4name.c u4util.c


library_includedir=$(includedir)/xbase
library_include_HEADERS= d4all.h

if BUILDING_FOX 
  lib_LTLIBRARIES= libxbase-fox.la 
  libxbase_fox_la_SOURCES=$SOURCES
endif

if BUILDING_MDX 
  lib_LTLIBRARIES= libxbase-mdx.la 
  libxbase_mdx_la_SOURCES=$SOURCES
endif

if BUILDING_NTX 
  lib_LTLIBRARIES= libxbase-clipper.la 
  libxbase_clipper_la_SOURCES=$SOURCES
endif


AM_LDFLAGS= -release $(VERSION) -shared -L$(top_builddir)/source/.libs

MODULE_FLAGS= -module -lxbase-$(VERSION) -no-undefined

if WINDOWS
  AM_CFLAGS=-DBUILDING_DLL
#  libxbase_la_SOURCES+= win32/winsock.c
#  libxbase_la_LDFLAGS= -LC:\windows\system32 ws2_32.dll -no-undefined
#  libxbase_la_LDFLAGS= -no-undefined -lgdi32

# xbase_udev_la_SOURCES= drivers/udev/udev-win.c
# xbase_udev_la_LDFLAGS= -uuid -lSetupAPI -lHid
else
#  libxbase_la_SOURCES+= unx/ioctl.c unx/os.c unx/select.c unx/signals.c unx/socks.c unx/gpio.c unx/rs232.c unx/piped.c unx/spi.c
# libbasela_LDFLAGS= -ldl

#  omad_udev_la_SOURCES= drivers/udev/udev-unx.c # unx/hid.c hidbnd.c
#  omad_udev_la_LDFLAGS= -ludev


#  lib_LTLIBRARIES+= omad-async.la
endif

