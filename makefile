
C1541=c1541

all: main.prg disk

main.prg:main.c
	cl65 -Osir -t c64 main.c -o main.prg

ned.pet: ned.txt
	petcat -text -w2 -o ned.pet -- ned.txt

disk: main.prg ned.pet
	$(C1541) -format ned,00 d64 ned.d64 \
		-write main.prg main \
		-write ned.pet

test:
	x64sc --autostart ned.d64

clean:
	$(RM) main.prg main.s main.o main.map main.lbl main.log main.lst
	$(RM) ned.d64
	$(RM) ned.pet
