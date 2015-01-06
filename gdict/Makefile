TGT=gdict
TGT_INSTALL=/usr/local/bin

all: ${TGT}

${TGT}: ${TGT}.c
	gcc -O2 -std=c99 -Wno-unused-result ${TGT}.c -o ${TGT}

install:
	install -d ${TGT_INSTALL}
	install -m 755 ${TGT} ${TGT_INSTALL}

uninstall:
	rm -f ${TGT_INSTALL}/${TGT}
	
clean:
	rm -f ${TGT}
