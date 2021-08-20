#!/usr/bin/env sh

# Copyright (C) 2021  Antonio Lassandro

# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.

# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e

NAME="Fang"
VERSION="0.0.0"

RUN=0
DEBUG=0
RELEASE=0

COMPILE_FLAGS=" "

while [ "$1" != "" ]; do
    case $1 in
        "help" )
            echo "\$./Scripts/macOS/Compile.sh [debug|release] [asan|ubsan] [run]"
            exit 0
            ;;

        "ubsan" )
            echo "Compiling with undefined behavior sanitizer..."
            COMPILE_FLAGS+="-fsanitize=undefined "
            ;;

        "asan" )
            echo "Compiling with address sanitizer..."
            COMPILE_FLAGS+="-fsanitize=address "
            ;;

        "release" )
            RELEASE=1
            ;;

        "debug" )
            DEBUG=1
            ;;

        "run" )
            RUN=1
            ;;

        * )
            COMPILE_FLAGS+="$1 "
            ;;
    esac
    shift
done

COMPILE_FLAGS+="-std=c11 "
COMPILE_FLAGS+="-pedantic "
COMPILE_FLAGS+="-Wall "
COMPILE_FLAGS+="-Wextra "
COMPILE_FLAGS+="-Wconversion "
COMPILE_FLAGS+="-Werror "
COMPILE_FLAGS+="-Wno-gnu-binary-literal "
COMPILE_FLAGS+="-Wlarge-by-value-copy "
COMPILE_FLAGS+="-Wkeyword-macro "
COMPILE_FLAGS+="-mmacosx-version-min=10.9 "
COMPILE_FLAGS+="-F/Library/Frameworks/ "
COMPILE_FLAGS+="-DFANG_TITLE=\"$NAME\" "
COMPILE_FLAGS+="-DFANG_VERSION=\"$VERSION\" "

DIR_BUILD="Build"

if test $RELEASE -eq 1; then
    COMPILE_FLAGS+="-O3 "
    COMPILE_FLAGS+="-Wframe-larger-than=4096 "
    DIR_BUILD+="/Release"
else
    COMPILE_FLAGS+="-g "
    COMPILE_FLAGS+="-Wno-unused-function "
    COMPILE_FLAGS+="-Wno-unused-label "
    COMPILE_FLAGS+="-Wframe-larger-than=8192 "
    DIR_BUILD+="/Debug"
fi

DIR_APP="$DIR_BUILD/$NAME.app"
DIR_CONTENTS="$DIR_APP/Contents"
DIR_MACOS="$DIR_CONTENTS/MacOS"
DIR_RESOURCES="$DIR_CONTENTS/Resources"
DIR_FRAMEWORKS="$DIR_CONTENTS/Frameworks"

BINARY="$DIR_MACOS/$NAME"
PLIST="$DIR_CONTENTS/Info.plist"

rm -rf "$DIR_BUILD"
mkdir -p "$DIR_MACOS"
mkdir -p "$DIR_RESOURCES"
mkdir -p "$DIR_FRAMEWORKS"

cp "Scripts/macOS/Info.plist" "$PLIST"
plutil -insert "CFBundleName" -string "$NAME" -s "$PLIST"
plutil -insert "CFBundleDisplayName" -string "$NAME" -s "$PLIST"
plutil -insert "CFBundleIdentifier" -string "com.lassandroan.$NAME" -s "$PLIST"
plutil -insert "CFBundleExecutable" -string "$NAME" -s "$PLIST"
plutil -insert "CFBundleVersion" -string "$VERISON" -s "$PLIST"
plutil -insert "CFBundleShortVersionString" -string "$VERISON" -s "$PLIST"
plutil -lint -s "$PLIST"

cp -r "Resources/" "$DIR_RESOURCES"

cc \
    $COMPILE_FLAGS \
    $(sdl2-config --cflags --libs) \
    -o "$BINARY" \
    "Source/Main.c"

if test $DEBUG -eq 1; then
    lldb -o run "./$BINARY"
elif test $RUN -eq 1; then
    "./$BINARY"
fi
