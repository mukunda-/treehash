# treehash
Treehash is a tool to detect directory changes. Say for example in your build file you have:

    sources( core/*.cpp )

Normally it isn't recommended to glob your source files, because when you commit changes to the source tree, other people on your team won't know they have to re-run the build file, and they'll get unexpected compilation errors.

That's what treehash is for, a fast way to check a source tree for any changes. After the buildsystem runs, it should save the source tree hash:

    $ treehash -e .cpp core*

Then in a pre-compilation event, the source tree can be hashed again and checked against the saved one, to make sure there aren't any new-or-removed files in the source tree, and re-run the build system if so. This is so you don't have to manually manage a list of files.

Worried about bogging-down compile times? That's what most people are worried about. Treehash is designed for speed, using one of the fastest hash algorithms available (www.xxhash.com). It walks your folders, hashes the filenames, adds them together, and prints the output. No fancy steps.

I can't say for certain if this is a good tool if your codebase is millions of files, but as an example for a moderate sized project:

    $ treehash Q:\work Q:\work\clang\* -t
    A072BF23A57AD676
    Time elapsed: 318ms

Hashing the ENTIRE clang folder takes about 318ms on my system (whereas compilation takes multiple hours). It might take some seconds longer on some passes if the filesystem needs to cache things, but that's a step that shouldn't have to be repeated during the compilation phase.

