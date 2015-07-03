CC		:= gcc
CFLAGS  := -Wall -lpthread -lm -g -finput-charset=UTF-8 -D_GNU_SOURCE=1 -DMONGOOSE_ENABLE_THREADS

SRCDIR	:= src/
OUTNAME := zncWebLogd

C_FILES 	 := $(wildcard $(SRCDIR)*.c) $(wildcard $(SRCDIR)*/*.c)
OBJ_FILES  	 := $(C_FILES:%.c=%.o)

main: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(OUTNAME) $^

debug: CFLAGS += -DDEBUG
debug:$(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(OUTNAME) $^  

PC: CFLAGS  += -DPC
PC: main
	@echo
	@echo Build will ignore hardeware errors
	@echo

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

rebuild: clean main
	
clean:
	find . -name '*.o' -delete
	rm -f $(OUTNAME)
	
install: $(OUTNAME)
	mkdir -p /etc/zncWebLog/HTML/
	touch /var/run/zwl.pid
	cp -r src/HTML/* /etc/zncWebLog/HTML/
	chmod 755 -R /etc/zncWebLog/
	cp zncWebLogd /usr/local/bin/
	cp ./install/init.d/zncWebLog /etc/init.d/
	chmod 755 /etc/init.d/zncWebLog
	cp ./install/config/zncWebLog.cfg /etc/zncWebLog/zncWebLog.cfg
	@echo adding rule to syslog
	cp ./install/rsyslog.d/30-zncWebLog.conf /etc/rsyslog.d/30-zncWebLog.conf
	service rsyslog restart
	@./install_config.sh
	
