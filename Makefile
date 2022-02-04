# * ---------------------
# * UTF-8 한글확인 용
# * ---------------------
 
.SUFFIXES = .cpp .o
CXX=g++

LIBUTIL = libamiutil.a
SRCS = util.cpp \
		logger.cpp \
		WebConfig.cpp \
		amiutilversion.cpp
OBJS = $(SRCS:.cpp=.o)

CFLAGS_RELEASE = -O2 -fPIC
CFLAGS_DEBUG = -O0 -g -ggdb3 -DDEBUG
CFLAGS  = -Wall -Wextra -Wshadow -Wformat-security -Winit-self -fpermissive
CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function

# 이 amiutil은 기본적으로 memcached, openssl, pthread 를 사용한다
LFLAGS  = -lm -ldl -lpthread -lmemcached -lcrypto

ifeq (release, $(findstring release, $(MAKECMDGOALS))) #"make clean release"으로 clean 다음에 연이어 release가 나올 수가 있다.
	CFLAGS += $(CFLAGS_RELEASE)
else
	CFLAGS += $(CFLAGS_DEBUG)
	LIBUTIL = libamiutil_d.a
ifeq (debugtrace, $(findstring debugtrace, $(MAKECMDGOALS))) #"make clean debugtrace"으로 clean 다음에 연이어 debugtrace 나올 수가 있다.
	CFLAGS += -DDEBUGTRACE
	LIBUTIL = libamiutil_t.a
endif
endif

DATE = $(shell date +'char amiutilCompileDate[20] = "%Y-%m-%d %H:%M:%S";')

.PHONY: all clean debug release debugtrace version

all: version $(LIBUTIL)
debug: all
release: all
debugtrace: all

%.o: %.cpp
	$(CXX) -o $@ $< -c $(CFLAGS)

version:
	echo '$(DATE)' > amiutilversion.cpp
	echo `$(CXX) -v`

$(LIBUTIL): $(OBJS)
	ar cq $(LIBUTIL) $(OBJS)
	mkdir -p ../lib
	cp $(LIBUTIL) ../lib
	mkdir -p ../include
	cp util.h ../include
	cp logger.h ../include
	cp WebConfig.h ../include


clean:
	rm -f $(LIBUTIL) *.o


