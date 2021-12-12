c:\gbdk\bin\png2mtspr spieler.png -sh 16 -spr8x8 -c spieler.c
c:\gbdk\bin\png2mtspr agent.png -sh 16 -spr8x8 -c agent.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o main.o main.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o largelevel.o largelevel.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o agent.o agent.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o spieler.o spieler.c
c:\gbdk\bin\lcc -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o stretchAttack.o stretchAttack.c
c:\gbdk\bin\bankpack -ext=.rel -v -yt19 main.o largelevel.o agent.o spieler.o stretchAttack.o
c:\gbdk\bin\lcc -Wa-l -Wl-m  -Wl-yt19 -Wl-yo4 -Wl-ya4 -o main.gb main.rel largelevel.rel agent.rel spieler.rel stretchAttack.rel