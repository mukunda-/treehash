
*(Please note that this is a work in progress and there are likely several bugs.)*
# treehash
Treehash is a tool to check for changes in a directory. Say for example in your build file you have:

    sources( core/*.cpp )

Normally it isn't recommended to glob your source files like that, because when you commit changes to the source tree, other people on your team won't know that they have to re-run the build file to update their compilation file list, and they'll get unexpected compilation errors.

Treehash is a fast way to check a source tree for any changes. After the buildsystem runs, it should save the source tree hash:

    $ treehash -e .cpp core*

Then in a pre-compilation event, the source tree can be hashed again and checked against the saved one, to make sure there aren't any new-or-removed files in the source tree, and re-run the build system if so. This is so you don't have to manually manage a list of individual source files. List whole folders in your buildsystem!

Worried about bogging-down compile times? You shouldn't. Treehash is designed for speed, using one of the fastest hash algorithms (www.xxhash.com). It's primitive; it walks your folders, hashes the filenames with simple filters, and adds them together. No fancy steps.

As an example for a moderate sized project:

    $ treehash.exe -b Q:\work Q:\work\llvm* Q:\work\clang* -t
    608888FEB9F4F3FA
    Time elapsed: 285ms

Hashing my copy of the Clang codebase takes about 285ms on my system, negligible compared to actual compilation times (multiple hours on my machine for fresh build). 99% of that time is the system calls; 1% is the hasher. The treehash might take some several seconds longer on some passes if the system needs to cache things about the files, but that doesn't matter because it's a step that needs to be done anyway for compilation.
