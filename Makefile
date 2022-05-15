CXX = g++
#CXXFLAGS = -Wall -Wextra -Wconversion -Werror -O2 -std=gnu++20
CXXFLAGS = -Wall -Wextra -O2 -std=gnu++17
CXX_SRC = robots-client

robots-client: Client.cpp Client.h Buffer.o ClientParameters.o GameInfo.o
	$(CXX) $(CXXFLAGS) -o $(CXX_SRC) Client.cpp Buffer.o ClientParameters.o GameInfo.o

ClientParameters.o: ClientParameters.cpp ClientParameters.h
	$(CXX) $(CXXFLAGS) -c ClientParameters.cpp

Buffer.o: Buffer.cpp Buffer.cpp Utils.h
	$(CXX) $(CXXFLAGS) -c Buffer.cpp

GameInfo.o: GameInfo.cpp GameInfo.h Utils.h
	$(CXX) $(CXXFLAGS) -c GameInfo.cpp

clean:
	rm -f *.o $(CXX_SRC)