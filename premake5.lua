-- treehash (C) 2019 Mukunda Johnson
-------------------------------------------------------------------------------
location  "build"
workspace "treehash"

configurations { "Debug", "Release" }
platforms      { "x86", "x64" }

filter "platforms:x86"
   architecture "x86"

filter "platforms:x64"
   architecture "x86_64"

filter {}

cppdialect "c++17"

configuration "Debug"
   defines  {"_DEBUG"}
   symbols  "On"
configuration "Release"
   defines  {"NDEBUG"}
   optimize "On"
   filter "action:vs*"
      buildoptions { "/Ob2", "/GL" }
      linkoptions  { "/LTCG:incremental" }
   filter {}
configuration {}

project   "treehash"
kind      "ConsoleApp"
language  "C++"

files { "source/*.cpp", "source/*.h" }
