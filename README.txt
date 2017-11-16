Dependencies:
    SDL2(2.0.7), SDL2_Image(2.0.2), SDL2_Mixer(2.0.1)

Note:
    SDL2_Mixer(2.0.2) will not initialise with ogg vorbis support.

To Build on windows:
    Download the above dependencies into the libs folder.
    run winbuild.bat
    Copy the required DLLs into the bin folder:
        libogg-0.dll
        libpng16-16.dll
        libvorbis-0.dll
        libvorbisfile-3.dll
        SDL2.dll
        SDL2_image.dll
        SDL2_mixer.dll
        zlib1.dll
    Copy (or make a symlink to) the res folder into the bin folder.

To Build on Linux:
    Install the following using your distro's package manager:
        libsdl2-dev
        libsdl2-image-dev
        libsdl2-mixer-dev
        clang 
    Give build.sh execution permissions
    Run build.sh
    Copy (or make soft link to) the res folder into the bin folder.

Controls:
    WASD:         Move forwards, left, back, and right
    Left Key:     Rotate camera left
    Right Key:    Rotate camera right
    Mouse:        Rotate camera
    Space:        Action (e.g. open doors)
    ALt-Enter:    Fullscreen
    P:            Pause
    Alt-F4 or Esc:Closes the game
