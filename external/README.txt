I had LOTS of issues building this as a subdirectory, and the license forces me to use dynamic linkin anyway
I compiled linux binaries myself, but win binaries were taken from github releases, so for safety I got two separate include files
linux binaries were just compiled using Release mode, no extra flags

OpenAL32.dll is actually 64 bits, this is what I had to do according to its readme (what the actual fuck I hate this)

also had the EXACT same issues with libsndfile and it also requires the .so to be shipped so whatever
just make sure to do something like cmake -DBUILD_SHARED_LIBS=on -DCMAKE_BUILD_TYPE=Release -S . -B build
