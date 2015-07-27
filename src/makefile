
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O2 -I. -I../storage -Wall -g
LFLAGS  = ../storage/libsophia.a -pthread -ldl -rdynamic
OBJECTS = sn_time.o \
          sn_config.o \
		  sn_log.o \
		  sn_module.o \
          sn_loop.o \
          sn_epoll.o \
          sn_socket.o \
          sn_client.o \
          sn_server.o \
		  sn_request.o \
		  sn_proc.o \
		  sn_cursor.o \
          sn_storage.o \
          sn.o \
          sn_execute.o \
		  serenity.o
TARGET  = serenity
$(TARGET): clean $(OBJECTS)
	@echo "ld $(TARGET)"
	@$(CC) $(OBJECTS) $(LFLAGS) -o ../$(TARGET)
.c.o:
	@$(CC) $(CFLAGS) -c $<
	@echo cc  $<
clean:
	@rm -f $(OBJECTS) $(TARGET)
