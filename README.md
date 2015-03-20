# QMikrotikAPI
A mikrotik ROS API writen with Qt5 SDK

It will be a API for communication between mikrotik's router boards and a proper Qt classes.

Current examples for such API (even C++ ones) that I've seen, uses blocking sockets and didn't takes care for
partial incoming data from network.
That's good for examples, but is unusable for real world.

This code tries to take the advantages from Qt libraries to create a clean and easy to understand code to be used
on any Qt project.

This API will just parse common and general word and sentence data comming from ROS (Routerboard OS) and will split it
and store into the correct classes to easy use. Will handle ROS protocol communication and login.
Even can store "query" words. But, won't do anything with "queries" as it's up to final application to create
the correct one "query" for ROS.
