
Introduction
------------

Yukon is a set of libraries and applications that are designed to capture
realtime videos of OpenGL applications (games).



Building
--------

Yukon depends on [seom][seom], and a few standard X11/OpenGL libraries. Other
than that, it's a standard configure+make project.

    ./configure && make && sudo make install

If you want to cross-compile, set CC on the make command-line.
For example, the following command-line will correctly install the 32bit
version of yukon in a 64bit userspace: 

    ./configure --libdir=lib32 && make CC="gcc -m32" && sudo make install



Running
-------

Explaining "how" yukon works can be little difficult -- therefore, we've
provided a wrapper script written in Bash that you can use to greatly
simplify matters. Just run the "yukon" script (installed into $PREFIX/bin)
with your application and it's parameters following. For example:

    yukon glxgears



Configuration
-------------

You'll probably want to use the yukon.conf file in the tools directory as
a starting point for you own configuration setups. However, the following
briefly describes which configuration files are parsed when Yukon is invoked.

 - /etc/yukon/conf
 - $HOME/.yukon/conf
 - $HOME/.yukon/programs/$APP
 - $HOME/.yukon/profiles/$PROFILE

$APP is the name of the executable (extracted from /proc/self/cmdline). You
may need to set VERBOSE=4 to see under which name your application is running.
$PROFILE is a profile name as selected using the --profile=??? option of the
yukon script.



Authors
-------

 - Tomas Carnecky (tomas.carnecky@gmail.com)
 - Jeremy L. Moles (jeremy@emperorlinux.com)
 - Michael Ploujnikov (ploujj@gmail.com)



[seom]: https://github.com/wereHamster/seom
