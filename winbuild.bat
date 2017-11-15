
@rem gcc -std=c99 -Wall -Werror -fmax-errors=1 -O3 src/*.c -IC:libs/SDL/include/SDL2 -LC:libs/SDL/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -o bin/wingame
clang -m32 -Ilibs/SDL2-2.0.7/include -Ilibs/SDL2_image-2.0.2/include -Ilibs/SDL2_mixer-2.0.1/include -Llibs/SDL2-2.0.7/lib/x86 -Llibs/SDL2_image-2.0.2/lib/x86 -Llibs/SDL2_mixer-2.0.1/lib/x86 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -Xlinker /subsystem:windows src/*.c  -o bin/oubliette.exe
@pause