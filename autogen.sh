#! /bin/sh

# Run this to generate all the auto-generated files needed by the GNU
# configure program

libtoolize --copy --force
aclocal
autoconf
automake --add-missing --copy --foreign
echo "Now you can run the configure script"
