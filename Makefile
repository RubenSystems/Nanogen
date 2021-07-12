# SOURCES=$(shell find ./src -name "*.cpp")

# TARGET=libnanogen.so
# CPPFLAGS=-std=c++2a -fPIC -g
# LDFLAGS= -shared


# .PHONY: all
# all: $(TARGET)


# %.o: %.cpp
#     $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

# # $(TARGET): $(OBJECTS)
# # 	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

# clean:
# 	rm -f $(TARGET) $(OBJECTS)

OBJECTS=$(SOURCES:%.cpp=%.o)
CXXFLAGS += -Wall -Wextra --std=c++2a
TARGET=libnanogen.a

src/%.o: src/%.cpp
    $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<


libnanogen.a: $(OBJECTS)
    ar crvs $@ $^

clean:
	rm -f $(TARGET) $(OBJECTS)