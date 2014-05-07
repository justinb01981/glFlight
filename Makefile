all:
	g++ -D_UNIX_=1 -lpthread -o httpfsd auth.cpp cpthread.cpp httpfileserver.cpp httpfsutils.cpp simplesocket.cpp
