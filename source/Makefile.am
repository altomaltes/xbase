lib_LTLIBRARIES= libnomad.la nomad-gsm.la nomad-cip.la nomad-mbus.la nomad-rfid.la nomad-vnc.la nomad-udev.la nomad-rs232.la nomad-thread.la

library_includedir=$(includedir)/altomaltes
library_include_HEADERS= nomad2.h

libnomad_la_SOURCES= inet.c net.c slot.c mux.c rs232.c pid.c

nomad_udev_la_CFLAGS=  -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_cip_la_CFLAGS=   -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_gsm_la_CFLAGS=   -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_vnc_la_CFLAGS=   -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_mbus_la_CFLAGS=  -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_rfid_la_CFLAGS=  -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_rs232_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_async_la_CFLAGS= -DBUILDING_SO_MODULE $(AM_CFLAGS)
nomad_thread_la_CFLAGS=  -DBUILDING_SO_MODULE $(AM_CFLAGS)

nomad_cip_la_SOURCES=   drivers/cip/ethip.c        drivers/cip/lgx.c  drivers/cip/pcl5.c
nomad_gsm_la_SOURCES=   drivers/gsm/hilink.c       drivers/gsm/html.c  drivers/gsm/sha256.c
nomad_mbus_la_SOURCES=  drivers/mbus/mbus.c        
nomad_vnc_la_SOURCES=   drivers/vnc/vnc.c
nomad_rfid_la_SOURCES=  drivers/rfid/rfid-rd232.c  drivers/rfid/rfid-usb.c  drivers/rfid/rfid.c  drivers/rfid/rfid-pn532.c drivers/rfid/rfid-guiliam.c
nomad_rs232_la_SOURCES= drivers/rs232/rain-RG15.c
nomad_thread_la_SOURCES= drivers/threads.c

LDFLAGS+= -release $(VERSION) -shared -L$(top_builddir)/src/.libs

MODULE_FLAGS= -module -lnomad-$(VERSION) -no-undefined

if WINDOWS
  AM_CFLAGS=-DBUILDING_DLL
  libnomad_la_SOURCES+= win32/winsock.c win32/file.c win32/gpio.c win32/os.c win32/piped.c win32/rs232.c win32/waitfor.c
  libnomad_la_LDFLAGS= -LC:\windows\system32 ws2_32.dll -no-undefined

  nomad_udev_la_SOURCES= drivers/udev/udev-win.c
  nomad_udev_la_LDFLAGS= -uuid -lSetupAPI -lHid
else
  libnomad_la_SOURCES+= unx/ioctl.c unx/os.c unx/select.c unx/signals.c unx/socks.c unx/gpio.c unx/rs232.c unx/piped.c unx/spi.c
  libnomad_la_LDFLAGS= -ldl

  nomad_udev_la_SOURCES= drivers/udev/udev-unx.c # unx/hid.c hidbnd.c
  nomad_udev_la_LDFLAGS= -ludev

  nomad_async_la_SOURCES= unx/async.c
  nomad_async_la_LDFLAGS= $(MODULE_FLAGS) -lrt

  lib_LTLIBRARIES+= nomad-async.la
endif

nomad_cip_la_LDFLAGS   = $(MODULE_FLAGS)
nomad_gsm_la_LDFLAGS   = $(MODULE_FLAGS) -lexpat
nomad_vnc_la_LDFLAGS   = $(MODULE_FLAGS) -lvncasync
nomad_mbus_la_LDFLAGS  = $(MODULE_FLAGS)
nomad_rfid_la_LDFLAGS  = $(MODULE_FLAGS)
nomad_rs232_la_LDFLAGS = $(MODULE_FLAGS)
nomad_thread_la_LDFLAGS= $(MODULE_FLAGS) -lpthread

nomad_udev_la_LDFLAGS+= $(MODULE_FLAGS)

