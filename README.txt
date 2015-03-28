This is a API to use with Mikrotik Router boards.

Read README.md for more detailed description.
here, you'll find how you can use it.

Using this code is quite easy. There is two ways to use:
1. If you want to play with example.
2. If you want just the API to your own project.

In both cases, you'll need Qt SDK and a compiler configured
to compile code from this library. The easiest way to obtain
Qt SKD and compile this API, is downloading Qt Creator for
your platform.

When you have the environment ready to complile Qt projects,
you're ready to compile this API.

First of, you shall clone or download project from Git
Then, to play with the example is as easy as open .pro file with QtCreator

But, if you just want API part of project is very easy too.
Just take Comm, QMD5 and QSentence (both .cpp and .h) source files,
COPYING, COPYING.LESSER files (for licence) and add it into your
project.
Then, you need to include network on your .pro file. Just add
"network" word (without quotes) in Qt statement. For example:

QT       += core gui network

To use this API is as easy as using this self-explained public
funtions and signals you can see in Comm.h.
You shall take a look to QSentence.h to see how to use this class
to more detailed control for sending sentences to ROS.
You can see a full example of how to use it in QMikApiExample files.

Remember that everything is under ROS namespace.
