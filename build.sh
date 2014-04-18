g++ -D_UNIX_=1 -pthread httpfileserver.cpp httpfsutils.cpp cpthread.cpp auth.cpp simplesocket.cpp -o httpfsd
