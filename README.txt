Dependencies:
    SDL2(2.0.7), SDL2_Image(2.0.0), SDL2_Mixer(2.0.2)

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

Controls:
    WASD:         Move forwards, left, back, and right
    Left Key:     Rotate camera left
    Right Key:    Rotate camera right
    Mouse:        Rotate camera
    Space:        Action (e.g. open doors)
    ALt-Enter:    Fullscreen
    P:            Pause
    Alt-F4 or Esc:Closes the game